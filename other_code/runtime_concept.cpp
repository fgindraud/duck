#include <memory>
#include <type_traits>
#include <utility>

#include <iostream>
#include <string>



#define TYPE_NAME(name)                                                                            \
	const char * type_name (const name &) { return #name; }
class Matrix;
TYPE_NAME (Matrix);
TYPE_NAME (int);
TYPE_NAME (std::string);

class Matrix {
public:
	Matrix () = delete;
	Matrix (Matrix &&) = default;
	Matrix & operator= (Matrix &&) = default;
	~Matrix () = default;

	template <typename T>
	using CanBuildMatrixFrom =
	    typename std::enable_if<!std::is_base_of<Matrix, typename std::decay<T>::type>::value>::type;

	template <typename T, typename = CanBuildMatrixFrom<T>>
	Matrix (T && t) : pImpl_ (std::make_unique<Model<T>> (std::forward<T> (t))) {}
	template <typename T, typename = CanBuildMatrixFrom<T>> Matrix & operator= (T && t) {
		pImpl_ = std::make_unique<Model<T>> (std::forward<T> (t));
	}

	Matrix (const Matrix & m) : pImpl_ (m.pImpl_->copy ()) {}
	Matrix & operator= (const Matrix & m) { pImpl_.reset (m.pImpl_->copy ()); }

	// Operations on objects
	friend std::ostream & operator<< (std::ostream & os, const Matrix & m) { m.pImpl_->print (os); }

private:
	struct Concept {
		virtual ~Concept () = default;
		virtual Concept * copy () const = 0;
		virtual void print (std::ostream & os) const = 0;
	};
	template <typename T> struct Model final : public Concept {
		Model (const T & t) : data_ (t) { std::cout << "copy_ctor " << type_name (data_) << std::endl; }
		Model (T && t) : data_ (std::move (t)) {
			std::cout << "move_ctor " << type_name (data_) << std::endl;
		}
		Model * copy () const override {
			std::cout << "copy " << type_name (data_) << std::endl;
			return new Model (*this);
		}
		void print (std::ostream & os) const override { os << data_; }
		T data_;
	};

	std::unique_ptr<Concept> pImpl_;
};

struct blah {};
TYPE_NAME (blah);
std::ostream & operator<< (std::ostream & os, const blah &) {
	return os << "blah blah blah";
}

int main () {
	Matrix a (42);
	Matrix b (Matrix (std::string ("hello")));
	Matrix c = blah{};
	Matrix d (std::move (a));
	std::cout << b << std::endl;
	std::cout << c << std::endl;
	std::cout << d << std::endl;
	return 0;
}
