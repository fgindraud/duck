#pragma once

// debug utilities

#include <iostream>

#ifdef DUCK_HAVE_DEMANGLING
#include <cstdlib>
#include <cxxabi.h>
#include <memory>
#endif

namespace duck {

inline std::string demangle (const char * name) {
#ifdef DUCK_HAVE_DEMANGLING
	int status{};
	std::unique_ptr<char, void (*) (void *)> res{
	    abi::__cxa_demangle (name, nullptr, nullptr, &status), std::free};
	return status == 0 ? res.get () : name;
#else
	return name;
#endif
}

template <char c = ' '> class Noisy {
public:
	Noisy () { msg ("default constr"); }
	Noisy (const Noisy &) { msg ("copy constr"); }
	Noisy (Noisy &&) noexcept { msg ("move constr"); }
	Noisy & operator= (const Noisy &) {
		msg ("copy assign");
		return *this;
	}
	Noisy & operator= (Noisy &&) noexcept {
		msg ("move assign");
		return *this;
	}
	~Noisy () { msg ("destr"); }

private:
	void msg (const char * operation) {
		std::cout << '[' << this << "] " << operation << ' ' << c << " noisy\n";
	}
};
}
