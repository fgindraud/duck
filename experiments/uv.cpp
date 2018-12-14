#include <cassert>
#include <exception>
#include <memory>
#include <uv.h>

#include <climits>
#include <cstdint>
#include <type_traits>

#if 0 // TODO general version with a weak_shared_ptr
template <typename Int, int N> struct IntegerWithBitFields {
	// The N last bits are used as bitfields.
	static_assert (std::is_unsigned<Int>::value);
	static constexpr Int integer_bit_size = sizeof (Int) * CHAR_BIT;
	static_assert (0 < N && N < integer_bit_size);
	static constexpr Int integer_bitmask = Int (-1) >> N;
	static constexpr Int bits_bitmask = ~integer_bitmask;
	template <int K> static constexpr Int bit_bitmask = Int (1) << (integer_bit_size - (N - K));

	Int value{0};

	Int get_int () const noexcept { return value & integer_bitmask; }
	void set_int (Int v) noexcept {
		assert (v <= integer_bitmask); // Detect overflows
		value = (value & bits_bitmask) | (v & integer_bitmask);
	}

	template <int K> bool get_bit () const noexcept {
		static_assert (0 <= K && K < N);
		return bool(value & bit_bitmask<K>);
	}
	template <int K> void set_bit (bool v) noexcept {
		static_assert (0 <= K && K < N);
		if (v) {
			value = value | bit_bitmask<K>;
		} else {
			value = value & ~bit_bitmask<K>;
		}
	}
};

template <typename T> struct DeletableSharedPtr {
	struct Block {
		IntegerWithBitFields<std::uint16_t, 1> reference_counter_and_built_bit;
		std::aligned_storage_t<sizeof (T), alignof (T)> storage;
	};
	Block * block{nullptr};
};
#endif

namespace uv {
// Basic exception type (no context)
struct Exception : std::exception {
	int error_code;
	Exception (int error_code) : error_code (error_code) {}
	const char * what () const noexcept final { return uv_strerror (error_code); }
};
inline void exception_on_error (int r) {
	if (r < 0) {
		throw Exception (r);
	}
}

/* Util struct:
 * Generate a default-constructible stateless closure type from a function pointer.
 * This is used to wrap stateless lambdas as other stateless lambdas.
 * Requires C++17 and clang.
 * In C++20 this will not be necessary (default constructible stateless lambdas).
 */
template <auto * f_ptr> struct ToFunctor {
	ToFunctor () = default;
	template <typename... Args> auto operator() (Args &&... args) const {
		return f_ptr (std::forward<Args> (args)...);
	}
};

// Basic event loop. Not copyable nor movable if it is referenced.
struct Loop : uv_loop_t {
	Loop () { exception_on_error (uv_loop_init (this)); }
	~Loop () { uv_loop_close (this); }

	Loop (const Loop &) = delete;
	Loop (Loop &&) = delete;
	Loop & operator= (const Loop &) = delete;
	Loop & operator= (Loop &&) = delete;

	static Loop & from (uv_loop_t & loop) noexcept { return static_cast<Loop &> (loop); }

	int run (uv_run_mode mode) { return uv_run (this, mode); }
	void stop () { uv_stop (this); }
};

template <typename UV_Handle> void destroy_handle (UV_Handle * uvh) noexcept {
	auto * handle = reinterpret_cast<uv_handle_t *> (uvh);
	uv_close (handle, [](uv_handle_t * handle) {
		assert (handle != nullptr);
		auto * uvh = reinterpret_cast<UV_Handle *> (handle);
		delete uvh;
	});
}

// Pointer to an inactive handle ; user has all rights.
template <typename UV_Handle> struct InactiveHandle;
// Pointer to an active handle, lent by UV in a callback.
// User can only choose to destroy it, but it cannot extend the lease or transfer it.
template <typename UV_Handle> struct ActiveHandleLease;

template <typename UV_Handle> struct InactiveHandle {
private:
	struct Deleter {
		void operator() (UV_Handle * uvh) const noexcept { destroy_handle (uvh); }
	};
	std::unique_ptr<UV_Handle, Deleter> ptr;

	explicit InactiveHandle (UV_Handle * uvh) noexcept : ptr (uvh) {}
	UV_Handle * handle () const noexcept { return ptr.get (); }
	UV_Handle * release () noexcept { return ptr.release (); }

	friend InactiveHandle<uv_tcp_t> create_tcp (Loop &);
	template <typename C> friend void listen (InactiveHandle<uv_tcp_t>, int, C);
	friend InactiveHandle<uv_tcp_t> accept (ActiveHandleLease<uv_tcp_t> &);
	friend void bind (InactiveHandle<uv_tcp_t> & tcp, const struct sockaddr & addr);

public:
	void close () noexcept { ptr.reset (); }
}; // namespace uv

template <typename UV_Handle> struct ActiveHandleLease {
private:
	UV_Handle * ptr;

	explicit ActiveHandleLease (UV_Handle * handle) : ptr (handle) { assert (ptr != nullptr); }
	UV_Handle * handle () const noexcept { return ptr; }
	void end () { ptr = nullptr; }

	template <typename C> friend void listen (InactiveHandle<uv_tcp_t>, int, C);
	friend InactiveHandle<uv_tcp_t> accept (ActiveHandleLease<uv_tcp_t> &);

public:
	ActiveHandleLease (const ActiveHandleLease &) = delete;
	ActiveHandleLease & operator= (const ActiveHandleLease &) = delete;
	ActiveHandleLease (ActiveHandleLease &&) = delete;
	ActiveHandleLease & operator= (ActiveHandleLease &&) = delete;
	~ActiveHandleLease () {
		if (ptr != nullptr) {
			destroy_handle (ptr);
		}
	}

	void close () noexcept {
		if (ptr != nullptr) {
			destroy_handle (ptr);
		}
	}
};

struct sockaddr_in ipv4_addr (const char * text_ip, int port) {
	struct sockaddr_in sin;
	exception_on_error (uv_ip4_addr (text_ip, port, &sin));
	return sin;
}

inline InactiveHandle<uv_tcp_t> create_tcp (Loop & loop) {
	auto storage = std::make_unique<uv_tcp_t> ();
	exception_on_error (uv_tcp_init (&loop, storage.get ()));
	return InactiveHandle<uv_tcp_t> (storage.release ());
}

inline void bind (InactiveHandle<uv_tcp_t> & tcp, const struct sockaddr & addr) {
	assert (tcp.handle ());
	exception_on_error (uv_tcp_bind (tcp.handle (), &addr, 0));
}
inline void bind (InactiveHandle<uv_tcp_t> & tcp, const struct sockaddr_in & addr) {
	bind (tcp, reinterpret_cast<const struct sockaddr &> (addr));
}

template <typename Callback>
void listen (InactiveHandle<uv_tcp_t> tcp, int backlog, Callback callback) {
	using CallbackType = void (*) (ActiveHandleLease<uv_tcp_t> &, int);
	using StatelessClosure = ToFunctor<CallbackType (callback)>;
	auto * stream = reinterpret_cast<uv_stream_t *> (tcp.handle ());
	exception_on_error (uv_listen (stream, backlog, [](uv_stream_t * stream, int status) {
		assert (stream != nullptr);
		// Temporarily take partial ownership from UV, lend it to the callback
		ActiveHandleLease<uv_tcp_t> lease (reinterpret_cast<uv_tcp_t *> (stream));
		StatelessClosure () (lease, status);
		lease.end ();
	}));
	tcp.release (); // Ownership in uv now
}

inline InactiveHandle<uv_tcp_t> accept (ActiveHandleLease<uv_tcp_t> & listener) {
	auto * listener_handle = listener.handle ();
	assert (listener_handle != nullptr);
	auto c = create_tcp (Loop::from (*listener_handle->loop));
	exception_on_error (uv_accept (reinterpret_cast<uv_stream_t *> (listener_handle),
	                               reinterpret_cast<uv_stream_t *> (c.handle ())));
	return c;
}
} // namespace uv

struct Database {
	// Stuff
};
Database load_database () {
	return {}; // Read file, etc
}

struct Service {
	const Database * database{nullptr};
};

int main () {
	const auto database = load_database ();

	uv::Loop loop;

	auto server = uv::create_tcp (loop);
	bind (server, uv::ipv4_addr ("0.0.0.0", 8000));
	listen (std::move (server), 10, [](auto & tcp, int status) {
		if (status < 0) {
			tcp.close ();
		} else {
			auto new_connection = accept (tcp);
			// Do nothing with it, it will be destroyed at the end of the callback
		}
	});

	return loop.run (UV_RUN_DEFAULT);
}
