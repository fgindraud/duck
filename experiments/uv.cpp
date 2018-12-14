#include <cassert>
#include <exception>
#include <memory>
#include <uv.h>

#include <climits>
#include <cstdint>
#include <type_traits>

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

template <typename T> struct DSP_Wrapper { std::int16_t reference_counter; };

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
 * Requires C++17 !
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

template <typename UV_Handle> struct WithRefCounter : UV_Handle {
	// Reference counter is used later
	int reference_counter = 0;
};

template <typename UV_Handle> struct UserOwned {
private:
	struct Deleter {
		void operator() (WithRefCounter<UV_Handle> * wrc) const {
			auto * uvh = static_cast<UV_Handle *> (wrc);
			auto * handle = reinterpret_cast<uv_handle_t *> (uvh);
			uv_close (handle, [](uv_handle_t * handle) {
				assert (handle != nullptr);
				auto * uvh = reinterpret_cast<UV_Handle *> (handle);
				auto * wrc = static_cast<WithRefCounter<UV_Handle> *> (uvh);
				delete wrc;
			});
		}
	};
	std::unique_ptr<WithRefCounter<UV_Handle>, Deleter> ptr;

public:
	explicit UserOwned (WithRefCounter<UV_Handle> * h) : ptr (h) {}
	UV_Handle * handle () const noexcept { return ptr.get (); }
};

// Weak_ptr like system TODO extract the pattern in separate class
template <typename UV_Handle> struct LoopOwned {
private:
	WithRefCounter<UV_Handle> * wrc;

public:
	UV_Handle * handle () const noexcept { return wrc; }
	int reference_counter () const noexcept {
		assert (wrc != nullptr);
		return wrc->reference_counter;
	}

	void close () {
		// Kill the connection, but do not delete memory until all other refs are done for
	}
};

struct sockaddr_in ipv4_addr (const char * text_ip, int port) {
	struct sockaddr_in sin;
	exception_on_error (uv_ip4_addr (text_ip, port, &sin));
	return sin;
}

inline UserOwned<uv_tcp_t> create_tcp (Loop & loop) {
	auto storage = std::make_unique<WithRefCounter<uv_tcp_t>> ();
	exception_on_error (uv_tcp_init (&loop, storage.get ()));
	return UserOwned<uv_tcp_t> (storage.release ());
}

inline void bind (UserOwned<uv_tcp_t> & tcp, const struct sockaddr & addr) {
	assert (tcp.handle ());
	exception_on_error (uv_tcp_bind (tcp.handle (), &addr, 0));
}
inline void bind (UserOwned<uv_tcp_t> & tcp, const struct sockaddr_in & addr) {
	bind (tcp, reinterpret_cast<const struct sockaddr &> (addr));
}

template <typename Callback>
LoopOwned<uv_tcp_t> listen (UserOwned<uv_tcp_t> && tcp, int backlog, Callback callback) {
	using StatelessClosure = ToFunctor<static_cast<void (*) (uv_tcp_t &, int)> (callback)>;
	auto * stream = reinterpret_cast<uv_stream_t *> (tcp.handle ());
	exception_on_error (uv_listen (stream, backlog, [](uv_stream_t * stream, int status) {
		assert (stream != nullptr);
		StatelessClosure () (reinterpret_cast<uv_tcp_t &> (*stream), status);
	}));
	return {};
}

#if 0
struct Tcp : private uv_tcp_t {
	UniquePtr accept () {
		auto c = create_unique (loop ());
		exception_on_error (uv_accept (&to_uv_stream (), &c->to_uv_stream ()));
		return c;
	}
};
#endif

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
	auto listener = listen (std::move (server), 10, [](uv_tcp_t & tcp, int) {
		// Do nothing
	});

	return loop.run (UV_RUN_DEFAULT);
}
