#include "bits.h"
#include "integer.h"
#include "range.h"
#include "types.h"
#include "raw_pointer.h"

#include <iostream>

int main () {
	std::cout << sizeof (BoundUint<65000>);
	return 0;
}
