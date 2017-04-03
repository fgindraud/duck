#include <iostream>
#include <type_traits>
#include <utility>

template <char c> class LogOperations {
public:
	LogOperations () { p ("default constr"); }
	LogOperations (const LogOperations &) { p ("copy constr"); }
	LogOperations (LogOperations &&) { p ("move constr"); }
	LogOperations & operator= (const LogOperations &) { p ("copy assign"); }
	LogOperations & operator= (LogOperations &&) { p ("move assign"); }
	~LogOperations () { p ("destr"); }

private:
	template <typename T> void p (T && t) {
		std::cout << '[' << c << '|' << static_cast<void *> (this) << "] " << std::forward<T> (t)
		          << '\n';
	}
};

struct Blah : public LogOperations<'B'> {
	int i_{42};
	Blah () = default;
	Blah (int i) : i_ (i) {}
};
Blah f (int i) {
	return Blah (i);
}

struct BlahPair : public LogOperations<'P'> {
	template <typename T, typename U,
	          typename = std::enable_if_t<std::is_constructible<Blah, T>::value>,
	          typename = std::enable_if_t<std::is_constructible<Blah, U>::value>>
	BlahPair (T && a, U && b) : a (std::forward<T> (a)), b (std::forward<U> (b)) {}
	Blah a;
	Blah b;
};

int main () {
	std::cout << "TEST 1\n";
	{ BlahPair p (f (1), Blah (34)); }
	std::cout << "TEST 2\n";
	{
		auto lv = Blah (0);
		BlahPair p (lv, Blah (34));
	}
	std::cout << "TEST 3\n";
	{
		BlahPair aaa (34, Blah (2));
	}
	// → Copy elision doesn't work for building pair.
	return 0;
}
