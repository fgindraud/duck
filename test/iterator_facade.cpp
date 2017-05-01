#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <duck/iterator/facade.h>
#include <duck/meta/maybe_type.h>
#include <type_traits>

struct Dummy {};

struct TypedefIterator {
	using value_type = Dummy;
	using reference = const value_type &;
	using pointer = const value_type *;
	using difference_type = Dummy;
};
struct NoTypedefIterator {
private:
	Dummy i;

public:
	const Dummy & deref () const { return i; }
};
struct NoTypedefRandomIterator : public NoTypedefIterator {
	int distance (NoTypedefRandomIterator) { return int(); }
};

TEST_CASE ("iterator typedef traits") {
	using namespace duck::Maybe;
	using namespace duck::Iterator::Detail;

	// Reference
	using ReferenceOfTypedefIteratorIsConstDummyRef =
	    std::is_same<Unpack<MaybeFacadeReferenceType<TypedefIterator>>, const Dummy &>;
	CHECK (ReferenceOfTypedefIteratorIsConstDummyRef::value);
	using ReferenceOfNoTypedefIteratorIsConstDummyRef =
	    std::is_same<Unpack<MaybeFacadeReferenceType<NoTypedefIterator>>, const Dummy &>;
	CHECK (ReferenceOfNoTypedefIteratorIsConstDummyRef::value);

	// ValueType
	using ValueTypeOfTypedefIteratorIsDummy =
	    std::is_same<Unpack<MaybeFacadeValueType<TypedefIterator>>, Dummy>;
	CHECK (ValueTypeOfTypedefIteratorIsDummy::value);
	using ValueTypeOfNoTypedefIteratorIsDummy =
	    std::is_same<Unpack<MaybeFacadeValueType<NoTypedefIterator>>, Dummy>;
	CHECK (ValueTypeOfNoTypedefIteratorIsDummy::value);

	// Pointer
	using PointerOfTypedefIteratorIsConstDummyRef =
	    std::is_same<Unpack<MaybeFacadePointerType<TypedefIterator>>, const Dummy *>;
	CHECK (PointerOfTypedefIteratorIsConstDummyRef::value);
	using PointerOfNoTypedefIteratorIsConstDummyRef =
	    std::is_same<Unpack<MaybeFacadePointerType<NoTypedefIterator>>, const Dummy *>;
	CHECK (PointerOfNoTypedefIteratorIsConstDummyRef::value);

	// DifferenceType
	using DifferenceTypeOfTypedefIteratorIsDummy =
	    std::is_same<Unpack<MaybeFacadeDifferenceType<TypedefIterator>>, Dummy>;
	CHECK (DifferenceTypeOfTypedefIteratorIsDummy::value);
	using DifferenceTypeOfNoTypedefIteratorIsPtrdiff =
	    std::is_same<Unpack<MaybeFacadeDifferenceType<NoTypedefIterator>>, std::ptrdiff_t>;
	CHECK (DifferenceTypeOfNoTypedefIteratorIsPtrdiff::value);
	using DifferenceTypeOfNoTypedefRandomIteratorIsInt =
	    std::is_same<Unpack<MaybeFacadeDifferenceType<NoTypedefRandomIterator>>, int>;
	CHECK (DifferenceTypeOfNoTypedefRandomIteratorIsInt::value);

	// MaybeDistance
	CHECK_FALSE (MaybeDistanceReturnType<TypedefIterator>::value);
	CHECK_FALSE (MaybeDistanceReturnType<NoTypedefIterator>::value);
	CHECK (MaybeDistanceReturnType<NoTypedefRandomIterator>::value);
	CHECK_FALSE (MaybeDistanceReturnType<int>::value);

	// MaybeDeref
	CHECK_FALSE (MaybeDerefReturnType<TypedefIterator>::value);
	CHECK (MaybeDerefReturnType<NoTypedefIterator>::value);
	CHECK (MaybeDerefReturnType<NoTypedefRandomIterator>::value);
	CHECK_FALSE (MaybeDerefReturnType<int>::value);
}

class IntItImpl {
public:
	IntItImpl (int i = 0) : i_ (i) {}
	void next () { advance (1); }
	void prev () { advance (-1); }
	bool equal (IntItImpl it) const { return distance (it) == 0; }
	int distance (IntItImpl it) const { return i_ - it.i_; }
	void advance (int n) { i_ += n; }
	const int & deref () const { return i_; }

private:
	int i_{};
};

using IntIt = duck::Iterator::Facade<IntItImpl>;

TEST_CASE ("random int iterator") {
	IntIt a{0};
	IntIt b{42};
	IntIt c;
	c = b;
	CHECK (b > a);
	CHECK ((b - a) == 42);
	b -= 32;
	CHECK (b == a + 10);
	CHECK (*a == 0);
	CHECK (*b == 10);
}

// TODO more tests
