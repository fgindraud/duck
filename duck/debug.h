#pragma once

// debug utilities

#include <iostream>

namespace duck {

class Noisy {
public:
	Noisy () { msg ("default constr"); }
	Noisy (const Noisy &) { msg ("copy constr"); }
	Noisy (Noisy &&) { msg ("move constr"); }
	Noisy & operator= (const Noisy &) { msg ("copy assign"); }
	Noisy & operator= (Noisy &&) { msg ("move assign"); }
	~Noisy () { msg ("destr"); }

private:
	void msg (const char * operation) { std::cout << '[' << this << "] " << operation << " noisy\n"; }
};
}
