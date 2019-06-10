
#ifndef _HLSL_H_
#define _HLSL_H_

#ifdef _MSC_VER
// NOTE: In MSVC10, <cmath> pollutes the global namespace.
#else
#include <cmath>
#endif

#ifdef min
#error min defined
#endif

#ifdef max
#error max defined
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4201) // non-standard extension: nameless struct / union
#elif __GNUG__
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

namespace hlsl {

/******************************************************************************/
/*                                   Types                                    */
/******************************************************************************/

template <typename T, int N>
struct vector;

/* Comparison */

template <typename T, int N>
bool operator==(const vector<T, N> & a, const vector<T, N> & b) {
	for (int i = 0; i < N; i++) {
		if (a[i] != b[i]) {
			return false;
		}
	}

	return true;
}

template <typename T, int N>
bool operator!=(const vector<T, N> & a, const vector<T, N> & b) {
	for (int i = 0; i < N; i++) {
		if (a[i] != b[i]) {
			return true;
		}
	}

	return false;
}

template <typename T, int N>
vector<bool, N> operator>(const vector<T, N> & a, const vector<T, N> & b) {
	vector<bool, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] > b[i];
	}
	return c;
}

template <typename T, int N>
vector<bool, N> operator>(const vector<T, N> & a, T b) {
	vector<bool, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] > b;
	}
	return c;
}

/* Arithmetic */

/* unary + */

template <typename T, int N>
inline vector<T, N> operator+(const vector<T, N> & a) {
	return a;
}

/* unary - */

template <typename T, int N>
vector<T, N> operator-(const vector<T, N> & a) {
	vector<T, N> b;
	for (int i = 0; i < N; i++) {
		b[i] = -a[i];
	}
	return b;
}

/* binary + */

template <typename T, int N>
vector<T, N> operator+(const vector<T, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] + b[i];
	}
	return c;
}

template <typename T, int N>
vector<T, N> operator+=(vector<T, N> & a, const vector<T, N> & b) {
	for (int i = 0; i < N; i++) {
		a[i] += b[i];
	}
	return a;
}

/* binary - */

template <typename T, int N>
vector<T, N> operator-(const vector<T, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] - b[i];
	}
	return c;
}

template <typename T, int N>
vector<T, N> operator-=(vector<T, N> & a, const vector<T, N> & b) {
	for (int i = 0; i < N; i++) {
		a[i] -= b[i];
	}
	return a;
}

/* binary * */

template <typename T, int N>
vector<T, N> operator*(const vector<T, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] * b[i];
	}
	return c;
}

template <typename T, int N>
vector<T, N> operator*(T a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a * b[i];
	}
	return c;
}

template <typename T, int N>
vector<T, N> operator*(const vector<T, N> & a, T b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] * b;
	}
	return c;
}

template <typename T, int N>
inline vector<T, N> operator*=(vector<T, N> & a, const vector<T, N> & b) {
	for (int i = 0; i < N; i++) {
		a[i] *= b[i];
	}
	return a;
}

template <typename T, int N>
inline vector<T, N> operator*=(vector<T, N> & a, T b) {
	for (int i = 0; i < N; i++) {
		a[i] *= b;
	}
	return a;
}

/* binary / */

template <typename T, int N>
vector<T, N> operator/(const vector<T, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] / b[i];
	}
	return c;
}

template <typename T, int N>
vector<T, N> operator/(T a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a / b[i];
	}
	return c;
}

template <typename T, int N>
vector<T, N> operator/(const vector<T, N> & a, T b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = a[i] / b;
	}
	return c;
}

template <typename T, int N>
inline vector<T, N> operator/=(vector<T, N> & a, const vector<T, N> & b) {
	for (int i = 0; i < N; i++) {
		a[i] /= b[i];
	}
	return a;
}

template <typename T, int N>
inline vector<T, N> operator/=(vector<T, N> & a, T b) {
	for (int i = 0; i < N; i++) {
		a[i] /= b;
	}
	return a;
}

/******************************************************************************/

template <typename T>
struct __vector_base {
	inline operator const T *() const {
		return (const T *) this;
	}

	inline operator T *() {
		return (T *) this;
	}
};

/******************************************************************************/

#define DEFINE_SWIZZLE_ONLY(__d__, __i__, __j__)\
template <typename T, int N>\
struct __##__i__##__j__ {\
	inline operator vector<T, __d__>() const {\
		const vector<T, N> & a = *(const vector<T, N> *) this;\
		return vector<T, __d__>(a.__i__, a.__j__);\
	}\
};

#define DEFINE_SWIZZLE_MASK(__d__, __i__, __j__)\
template <typename T, int N>\
struct __##__i__##__j__ {\
	inline __##__i__##__j__ & operator=(const vector<T, __d__> & b) {\
		if (this != &b.__i__##__j__) {\
			vector<T, N> & a = *(vector<T, N> *) this;\
			a.__i__ = b.x;\
			a.__j__ = b.y;\
		}\
		return *this;\
	}\
	inline operator vector<T, __d__>() const {\
		const vector<T, N> & a = *(const vector<T, N> *) this;\
		return vector<T, __d__>(a.__i__, a.__j__);\
	}\
};\
\
template <typename T, int N>\
vector<T, N> operator-(const vector<T, N> & a, const __##__i__##__j__<T, N> & b) {\
	return operator-(a, (vector<T, N>) b);\
}

DEFINE_SWIZZLE_ONLY(2, x, x)
DEFINE_SWIZZLE_MASK(2, x, y)
DEFINE_SWIZZLE_MASK(2, x, z)
DEFINE_SWIZZLE_MASK(2, x, w)
DEFINE_SWIZZLE_MASK(2, y, x)
DEFINE_SWIZZLE_ONLY(2, y, y)
DEFINE_SWIZZLE_MASK(2, y, z)
DEFINE_SWIZZLE_MASK(2, y, w)
DEFINE_SWIZZLE_MASK(2, z, x)
DEFINE_SWIZZLE_MASK(2, z, y)
DEFINE_SWIZZLE_ONLY(2, z, z)
DEFINE_SWIZZLE_MASK(2, z, w)
DEFINE_SWIZZLE_MASK(2, w, x)
DEFINE_SWIZZLE_MASK(2, w, y)
DEFINE_SWIZZLE_MASK(2, w, z)
DEFINE_SWIZZLE_ONLY(2, w, w)

#undef DEFINE_SWIZZLE_ONLY
#undef DEFINE_SWIZZLE_MASK

#define DEFINE_SWIZZLE_ONLY(__d__, __i__, __j__, __k__)\
	template <typename T, int N>\
struct __##__i__##__j__##__k__ {\
	inline operator vector<T, __d__>() const {\
		const vector<T, N> & a = *(const vector<T, N> *) this;\
		return vector<T, __d__>(a.__i__, a.__j__, a.__k__);\
	}\
};

#define DEFINE_SWIZZLE_MASK(__d__, __i__, __j__, __k__)\
	template <typename T, int N>\
struct __##__i__##__j__##__k__ {\
	inline __##__i__##__j__##__k__ & operator=(const vector<T, __d__> & b) {\
	if (this != &b.__i__##__j__##__k__) {\
			vector<T, N> & a = *(vector<T, N> *) this;\
			a.__i__ = b.x;\
			a.__j__ = b.y;\
			a.__k__ = b.z;\
		}\
		return *this;\
	}\
	inline operator vector<T, __d__>() const {\
		const vector<T, N> & a = *(const vector<T, N> *) this;\
		return vector<T, __d__>(a.__i__, a.__j__, a.__k__);\
	}\
};

DEFINE_SWIZZLE_ONLY(3, x, x, x)
DEFINE_SWIZZLE_ONLY(3, x, x, y)
DEFINE_SWIZZLE_ONLY(3, x, x, z)
DEFINE_SWIZZLE_ONLY(3, x, x, w)
DEFINE_SWIZZLE_ONLY(3, x, y, x)
DEFINE_SWIZZLE_ONLY(3, x, y, y)
DEFINE_SWIZZLE_MASK(3, x, y, z)
DEFINE_SWIZZLE_MASK(3, x, y, w)
DEFINE_SWIZZLE_ONLY(3, x, z, x)
DEFINE_SWIZZLE_MASK(3, x, z, y)
DEFINE_SWIZZLE_ONLY(3, x, z, z)
DEFINE_SWIZZLE_MASK(3, x, z, w)
DEFINE_SWIZZLE_ONLY(3, x, w, x)
DEFINE_SWIZZLE_MASK(3, x, w, y)
DEFINE_SWIZZLE_MASK(3, x, w, z)
DEFINE_SWIZZLE_ONLY(3, x, w, w)
DEFINE_SWIZZLE_ONLY(3, y, x, x)
DEFINE_SWIZZLE_ONLY(3, y, x, y)
DEFINE_SWIZZLE_MASK(3, y, x, z)
DEFINE_SWIZZLE_MASK(3, y, x, w)
DEFINE_SWIZZLE_ONLY(3, y, y, x)
DEFINE_SWIZZLE_ONLY(3, y, y, y)
DEFINE_SWIZZLE_ONLY(3, y, y, z)
DEFINE_SWIZZLE_ONLY(3, y, y, w)
DEFINE_SWIZZLE_MASK(3, y, z, x)
DEFINE_SWIZZLE_ONLY(3, y, z, y)
DEFINE_SWIZZLE_ONLY(3, y, z, z)
DEFINE_SWIZZLE_MASK(3, y, z, w)
DEFINE_SWIZZLE_MASK(3, y, w, x)
DEFINE_SWIZZLE_ONLY(3, y, w, y)
DEFINE_SWIZZLE_MASK(3, y, w, z)
DEFINE_SWIZZLE_ONLY(3, y, w, w)
DEFINE_SWIZZLE_ONLY(3, z, x, x)
DEFINE_SWIZZLE_MASK(3, z, x, y)
DEFINE_SWIZZLE_ONLY(3, z, x, z)
DEFINE_SWIZZLE_MASK(3, z, x, w)
DEFINE_SWIZZLE_MASK(3, z, y, x)
DEFINE_SWIZZLE_ONLY(3, z, y, y)
DEFINE_SWIZZLE_ONLY(3, z, y, z)
DEFINE_SWIZZLE_MASK(3, z, y, w)
DEFINE_SWIZZLE_ONLY(3, z, z, x)
DEFINE_SWIZZLE_ONLY(3, z, z, y)
DEFINE_SWIZZLE_ONLY(3, z, z, z)
DEFINE_SWIZZLE_ONLY(3, z, z, w)
DEFINE_SWIZZLE_MASK(3, z, w, x)
DEFINE_SWIZZLE_MASK(3, z, w, y)
DEFINE_SWIZZLE_ONLY(3, z, w, z)
DEFINE_SWIZZLE_ONLY(3, z, w, w)
DEFINE_SWIZZLE_ONLY(3, w, x, x)
DEFINE_SWIZZLE_MASK(3, w, x, y)
DEFINE_SWIZZLE_MASK(3, w, x, z)
DEFINE_SWIZZLE_ONLY(3, w, x, w)
DEFINE_SWIZZLE_MASK(3, w, y, x)
DEFINE_SWIZZLE_ONLY(3, w, y, y)
DEFINE_SWIZZLE_MASK(3, w, y, z)
DEFINE_SWIZZLE_ONLY(3, w, y, w)
DEFINE_SWIZZLE_MASK(3, w, z, x)
DEFINE_SWIZZLE_MASK(3, w, z, y)
DEFINE_SWIZZLE_ONLY(3, w, z, z)
DEFINE_SWIZZLE_ONLY(3, w, z, w)
DEFINE_SWIZZLE_ONLY(3, w, w, x)
DEFINE_SWIZZLE_ONLY(3, w, w, y)
DEFINE_SWIZZLE_ONLY(3, w, w, z)
DEFINE_SWIZZLE_ONLY(3, w, w, w)

#undef DEFINE_SWIZZLE_ONLY
#undef DEFINE_SWIZZLE_MASK

#define DEFINE_SWIZZLE_ONLY(__d__, __i__, __j__, __k__, __l__)\
template <typename T, int N>\
struct __##__i__##__j__##__k__##__l__ {\
	inline operator vector<T, __d__>() const {\
		const vector<T, N> & a = *(const vector<T, N> *) this;\
		return vector<T, __d__>(a.__i__, a.__j__, a.__k__, a.__l__);\
	}\
};

#define DEFINE_SWIZZLE_MASK(__d__, __i__, __j__, __k__, __l__)\
template <typename T, int N>\
struct __##__i__##__j__##__k__##__l__ {\
	inline __##__i__##__j__##__k__##__l__ & operator=(const vector<T, __d__> & b) {\
		if (this != &b.__i__##__j__##__k__##__l__) {\
			vector<T, N> & a = *(vector<T, N> *) this;\
			a.__i__ = b.x;\
			a.__j__ = b.y;\
			a.__k__ = b.z;\
			a.__l__ = b.w;\
		}\
		return *this;\
	}\
	inline operator const vector<T, __d__>() const {\
		const vector<T, N> & a = *(const vector<T, N> *) this;\
		return vector<T, __d__>(a.__i__, a.__j__, a.__k__, a.__l__);\
	}\
};

DEFINE_SWIZZLE_ONLY(4, x, x, x, x)
DEFINE_SWIZZLE_ONLY(4, x, x, x, y)
DEFINE_SWIZZLE_ONLY(4, x, x, x, z)
DEFINE_SWIZZLE_ONLY(4, x, x, x, w)
DEFINE_SWIZZLE_ONLY(4, x, x, y, x)
DEFINE_SWIZZLE_ONLY(4, x, x, y, y)
DEFINE_SWIZZLE_ONLY(4, x, x, y, z)
DEFINE_SWIZZLE_ONLY(4, x, x, y, w)
DEFINE_SWIZZLE_ONLY(4, x, x, z, x)
DEFINE_SWIZZLE_ONLY(4, x, x, z, y)
DEFINE_SWIZZLE_ONLY(4, x, x, z, z)
DEFINE_SWIZZLE_ONLY(4, x, x, z, w)
DEFINE_SWIZZLE_ONLY(4, x, x, w, x)
DEFINE_SWIZZLE_ONLY(4, x, x, w, y)
DEFINE_SWIZZLE_ONLY(4, x, x, w, z)
DEFINE_SWIZZLE_ONLY(4, x, x, w, w)
DEFINE_SWIZZLE_ONLY(4, x, y, x, x)
DEFINE_SWIZZLE_ONLY(4, x, y, x, y)
DEFINE_SWIZZLE_ONLY(4, x, y, x, z)
DEFINE_SWIZZLE_ONLY(4, x, y, x, w)
DEFINE_SWIZZLE_ONLY(4, x, y, y, x)
DEFINE_SWIZZLE_ONLY(4, x, y, y, y)
DEFINE_SWIZZLE_ONLY(4, x, y, y, z)
DEFINE_SWIZZLE_ONLY(4, x, y, y, w)
DEFINE_SWIZZLE_ONLY(4, x, y, z, x)
DEFINE_SWIZZLE_ONLY(4, x, y, z, y)
DEFINE_SWIZZLE_ONLY(4, x, y, z, z)
DEFINE_SWIZZLE_MASK(4, x, y, z, w)
DEFINE_SWIZZLE_ONLY(4, x, y, w, x)
DEFINE_SWIZZLE_ONLY(4, x, y, w, y)
DEFINE_SWIZZLE_MASK(4, x, y, w, z)
DEFINE_SWIZZLE_ONLY(4, x, y, w, w)
DEFINE_SWIZZLE_ONLY(4, x, z, x, x)
DEFINE_SWIZZLE_ONLY(4, x, z, x, y)
DEFINE_SWIZZLE_ONLY(4, x, z, x, z)
DEFINE_SWIZZLE_ONLY(4, x, z, x, w)
DEFINE_SWIZZLE_ONLY(4, x, z, y, x)
DEFINE_SWIZZLE_ONLY(4, x, z, y, y)
DEFINE_SWIZZLE_ONLY(4, x, z, y, z)
DEFINE_SWIZZLE_MASK(4, x, z, y, w)
DEFINE_SWIZZLE_ONLY(4, x, z, z, x)
DEFINE_SWIZZLE_ONLY(4, x, z, z, y)
DEFINE_SWIZZLE_ONLY(4, x, z, z, z)
DEFINE_SWIZZLE_ONLY(4, x, z, z, w)
DEFINE_SWIZZLE_ONLY(4, x, z, w, x)
DEFINE_SWIZZLE_MASK(4, x, z, w, y)
DEFINE_SWIZZLE_ONLY(4, x, z, w, z)
DEFINE_SWIZZLE_ONLY(4, x, z, w, w)
DEFINE_SWIZZLE_ONLY(4, x, w, x, x)
DEFINE_SWIZZLE_ONLY(4, x, w, x, y)
DEFINE_SWIZZLE_ONLY(4, x, w, x, z)
DEFINE_SWIZZLE_ONLY(4, x, w, x, w)
DEFINE_SWIZZLE_ONLY(4, x, w, y, x)
DEFINE_SWIZZLE_ONLY(4, x, w, y, y)
DEFINE_SWIZZLE_MASK(4, x, w, y, z)
DEFINE_SWIZZLE_ONLY(4, x, w, y, w)
DEFINE_SWIZZLE_ONLY(4, x, w, z, x)
DEFINE_SWIZZLE_MASK(4, x, w, z, y)
DEFINE_SWIZZLE_ONLY(4, x, w, z, z)
DEFINE_SWIZZLE_ONLY(4, x, w, z, w)
DEFINE_SWIZZLE_ONLY(4, x, w, w, x)
DEFINE_SWIZZLE_ONLY(4, x, w, w, y)
DEFINE_SWIZZLE_ONLY(4, x, w, w, z)
DEFINE_SWIZZLE_ONLY(4, x, w, w, w)
DEFINE_SWIZZLE_ONLY(4, y, x, x, x)
DEFINE_SWIZZLE_ONLY(4, y, x, x, y)
DEFINE_SWIZZLE_ONLY(4, y, x, x, z)
DEFINE_SWIZZLE_ONLY(4, y, x, x, w)
DEFINE_SWIZZLE_ONLY(4, y, x, y, x)
DEFINE_SWIZZLE_ONLY(4, y, x, y, y)
DEFINE_SWIZZLE_ONLY(4, y, x, y, z)
DEFINE_SWIZZLE_ONLY(4, y, x, y, w)
DEFINE_SWIZZLE_ONLY(4, y, x, z, x)
DEFINE_SWIZZLE_ONLY(4, y, x, z, y)
DEFINE_SWIZZLE_ONLY(4, y, x, z, z)
DEFINE_SWIZZLE_MASK(4, y, x, z, w)
DEFINE_SWIZZLE_ONLY(4, y, x, w, x)
DEFINE_SWIZZLE_ONLY(4, y, x, w, y)
DEFINE_SWIZZLE_MASK(4, y, x, w, z)
DEFINE_SWIZZLE_ONLY(4, y, x, w, w)
DEFINE_SWIZZLE_ONLY(4, y, y, x, x)
DEFINE_SWIZZLE_ONLY(4, y, y, x, y)
DEFINE_SWIZZLE_ONLY(4, y, y, x, z)
DEFINE_SWIZZLE_ONLY(4, y, y, x, w)
DEFINE_SWIZZLE_ONLY(4, y, y, y, x)
DEFINE_SWIZZLE_ONLY(4, y, y, y, y)
DEFINE_SWIZZLE_ONLY(4, y, y, y, z)
DEFINE_SWIZZLE_ONLY(4, y, y, y, w)
DEFINE_SWIZZLE_ONLY(4, y, y, z, x)
DEFINE_SWIZZLE_ONLY(4, y, y, z, y)
DEFINE_SWIZZLE_ONLY(4, y, y, z, z)
DEFINE_SWIZZLE_ONLY(4, y, y, z, w)
DEFINE_SWIZZLE_ONLY(4, y, y, w, x)
DEFINE_SWIZZLE_ONLY(4, y, y, w, y)
DEFINE_SWIZZLE_ONLY(4, y, y, w, z)
DEFINE_SWIZZLE_ONLY(4, y, y, w, w)
DEFINE_SWIZZLE_ONLY(4, y, z, x, x)
DEFINE_SWIZZLE_ONLY(4, y, z, x, y)
DEFINE_SWIZZLE_ONLY(4, y, z, x, z)
DEFINE_SWIZZLE_MASK(4, y, z, x, w)
DEFINE_SWIZZLE_ONLY(4, y, z, y, x)
DEFINE_SWIZZLE_ONLY(4, y, z, y, y)
DEFINE_SWIZZLE_ONLY(4, y, z, y, z)
DEFINE_SWIZZLE_ONLY(4, y, z, y, w)
DEFINE_SWIZZLE_ONLY(4, y, z, z, x)
DEFINE_SWIZZLE_ONLY(4, y, z, z, y)
DEFINE_SWIZZLE_ONLY(4, y, z, z, z)
DEFINE_SWIZZLE_ONLY(4, y, z, z, w)
DEFINE_SWIZZLE_MASK(4, y, z, w, x)
DEFINE_SWIZZLE_ONLY(4, y, z, w, y)
DEFINE_SWIZZLE_ONLY(4, y, z, w, z)
DEFINE_SWIZZLE_ONLY(4, y, z, w, w)
DEFINE_SWIZZLE_ONLY(4, y, w, x, x)
DEFINE_SWIZZLE_ONLY(4, y, w, x, y)
DEFINE_SWIZZLE_MASK(4, y, w, x, z)
DEFINE_SWIZZLE_ONLY(4, y, w, x, w)
DEFINE_SWIZZLE_ONLY(4, y, w, y, x)
DEFINE_SWIZZLE_ONLY(4, y, w, y, y)
DEFINE_SWIZZLE_ONLY(4, y, w, y, z)
DEFINE_SWIZZLE_ONLY(4, y, w, y, w)
DEFINE_SWIZZLE_MASK(4, y, w, z, x)
DEFINE_SWIZZLE_ONLY(4, y, w, z, y)
DEFINE_SWIZZLE_ONLY(4, y, w, z, z)
DEFINE_SWIZZLE_ONLY(4, y, w, z, w)
DEFINE_SWIZZLE_ONLY(4, y, w, w, x)
DEFINE_SWIZZLE_ONLY(4, y, w, w, y)
DEFINE_SWIZZLE_ONLY(4, y, w, w, z)
DEFINE_SWIZZLE_ONLY(4, y, w, w, w)
DEFINE_SWIZZLE_ONLY(4, z, x, x, x)
DEFINE_SWIZZLE_ONLY(4, z, x, x, y)
DEFINE_SWIZZLE_ONLY(4, z, x, x, z)
DEFINE_SWIZZLE_ONLY(4, z, x, x, w)
DEFINE_SWIZZLE_ONLY(4, z, x, y, x)
DEFINE_SWIZZLE_ONLY(4, z, x, y, y)
DEFINE_SWIZZLE_ONLY(4, z, x, y, z)
DEFINE_SWIZZLE_MASK(4, z, x, y, w)
DEFINE_SWIZZLE_ONLY(4, z, x, z, x)
DEFINE_SWIZZLE_ONLY(4, z, x, z, y)
DEFINE_SWIZZLE_ONLY(4, z, x, z, z)
DEFINE_SWIZZLE_ONLY(4, z, x, z, w)
DEFINE_SWIZZLE_ONLY(4, z, x, w, x)
DEFINE_SWIZZLE_MASK(4, z, x, w, y)
DEFINE_SWIZZLE_ONLY(4, z, x, w, z)
DEFINE_SWIZZLE_ONLY(4, z, x, w, w)
DEFINE_SWIZZLE_ONLY(4, z, y, x, x)
DEFINE_SWIZZLE_ONLY(4, z, y, x, y)
DEFINE_SWIZZLE_ONLY(4, z, y, x, z)
DEFINE_SWIZZLE_MASK(4, z, y, x, w)
DEFINE_SWIZZLE_ONLY(4, z, y, y, x)
DEFINE_SWIZZLE_ONLY(4, z, y, y, y)
DEFINE_SWIZZLE_ONLY(4, z, y, y, z)
DEFINE_SWIZZLE_ONLY(4, z, y, y, w)
DEFINE_SWIZZLE_ONLY(4, z, y, z, x)
DEFINE_SWIZZLE_ONLY(4, z, y, z, y)
DEFINE_SWIZZLE_ONLY(4, z, y, z, z)
DEFINE_SWIZZLE_ONLY(4, z, y, z, w)
DEFINE_SWIZZLE_MASK(4, z, y, w, x)
DEFINE_SWIZZLE_ONLY(4, z, y, w, y)
DEFINE_SWIZZLE_ONLY(4, z, y, w, z)
DEFINE_SWIZZLE_ONLY(4, z, y, w, w)
DEFINE_SWIZZLE_ONLY(4, z, z, x, x)
DEFINE_SWIZZLE_ONLY(4, z, z, x, y)
DEFINE_SWIZZLE_ONLY(4, z, z, x, z)
DEFINE_SWIZZLE_ONLY(4, z, z, x, w)
DEFINE_SWIZZLE_ONLY(4, z, z, y, x)
DEFINE_SWIZZLE_ONLY(4, z, z, y, y)
DEFINE_SWIZZLE_ONLY(4, z, z, y, z)
DEFINE_SWIZZLE_ONLY(4, z, z, y, w)
DEFINE_SWIZZLE_ONLY(4, z, z, z, x)
DEFINE_SWIZZLE_ONLY(4, z, z, z, y)
DEFINE_SWIZZLE_ONLY(4, z, z, z, z)
DEFINE_SWIZZLE_ONLY(4, z, z, z, w)
DEFINE_SWIZZLE_ONLY(4, z, z, w, x)
DEFINE_SWIZZLE_ONLY(4, z, z, w, y)
DEFINE_SWIZZLE_ONLY(4, z, z, w, z)
DEFINE_SWIZZLE_ONLY(4, z, z, w, w)
DEFINE_SWIZZLE_ONLY(4, z, w, x, x)
DEFINE_SWIZZLE_MASK(4, z, w, x, y)
DEFINE_SWIZZLE_ONLY(4, z, w, x, z)
DEFINE_SWIZZLE_ONLY(4, z, w, x, w)
DEFINE_SWIZZLE_MASK(4, z, w, y, x)
DEFINE_SWIZZLE_ONLY(4, z, w, y, y)
DEFINE_SWIZZLE_ONLY(4, z, w, y, z)
DEFINE_SWIZZLE_ONLY(4, z, w, y, w)
DEFINE_SWIZZLE_ONLY(4, z, w, z, x)
DEFINE_SWIZZLE_ONLY(4, z, w, z, y)
DEFINE_SWIZZLE_ONLY(4, z, w, z, z)
DEFINE_SWIZZLE_ONLY(4, z, w, z, w)
DEFINE_SWIZZLE_ONLY(4, z, w, w, x)
DEFINE_SWIZZLE_ONLY(4, z, w, w, y)
DEFINE_SWIZZLE_ONLY(4, z, w, w, z)
DEFINE_SWIZZLE_ONLY(4, z, w, w, w)
DEFINE_SWIZZLE_ONLY(4, w, x, x, x)
DEFINE_SWIZZLE_ONLY(4, w, x, x, y)
DEFINE_SWIZZLE_ONLY(4, w, x, x, z)
DEFINE_SWIZZLE_ONLY(4, w, x, x, w)
DEFINE_SWIZZLE_ONLY(4, w, x, y, x)
DEFINE_SWIZZLE_ONLY(4, w, x, y, y)
DEFINE_SWIZZLE_MASK(4, w, x, y, z)
DEFINE_SWIZZLE_ONLY(4, w, x, y, w)
DEFINE_SWIZZLE_ONLY(4, w, x, z, x)
DEFINE_SWIZZLE_MASK(4, w, x, z, y)
DEFINE_SWIZZLE_ONLY(4, w, x, z, z)
DEFINE_SWIZZLE_ONLY(4, w, x, z, w)
DEFINE_SWIZZLE_ONLY(4, w, x, w, x)
DEFINE_SWIZZLE_ONLY(4, w, x, w, y)
DEFINE_SWIZZLE_ONLY(4, w, x, w, z)
DEFINE_SWIZZLE_ONLY(4, w, x, w, w)
DEFINE_SWIZZLE_ONLY(4, w, y, x, x)
DEFINE_SWIZZLE_ONLY(4, w, y, x, y)
DEFINE_SWIZZLE_MASK(4, w, y, x, z)
DEFINE_SWIZZLE_ONLY(4, w, y, x, w)
DEFINE_SWIZZLE_ONLY(4, w, y, y, x)
DEFINE_SWIZZLE_ONLY(4, w, y, y, y)
DEFINE_SWIZZLE_ONLY(4, w, y, y, z)
DEFINE_SWIZZLE_ONLY(4, w, y, y, w)
DEFINE_SWIZZLE_MASK(4, w, y, z, x)
DEFINE_SWIZZLE_ONLY(4, w, y, z, y)
DEFINE_SWIZZLE_ONLY(4, w, y, z, z)
DEFINE_SWIZZLE_ONLY(4, w, y, z, w)
DEFINE_SWIZZLE_ONLY(4, w, y, w, x)
DEFINE_SWIZZLE_ONLY(4, w, y, w, y)
DEFINE_SWIZZLE_ONLY(4, w, y, w, z)
DEFINE_SWIZZLE_ONLY(4, w, y, w, w)
DEFINE_SWIZZLE_ONLY(4, w, z, x, x)
DEFINE_SWIZZLE_MASK(4, w, z, x, y)
DEFINE_SWIZZLE_ONLY(4, w, z, x, z)
DEFINE_SWIZZLE_ONLY(4, w, z, x, w)
DEFINE_SWIZZLE_MASK(4, w, z, y, x)
DEFINE_SWIZZLE_ONLY(4, w, z, y, y)
DEFINE_SWIZZLE_ONLY(4, w, z, y, z)
DEFINE_SWIZZLE_ONLY(4, w, z, y, w)
DEFINE_SWIZZLE_ONLY(4, w, z, z, x)
DEFINE_SWIZZLE_ONLY(4, w, z, z, y)
DEFINE_SWIZZLE_ONLY(4, w, z, z, z)
DEFINE_SWIZZLE_ONLY(4, w, z, z, w)
DEFINE_SWIZZLE_ONLY(4, w, z, w, x)
DEFINE_SWIZZLE_ONLY(4, w, z, w, y)
DEFINE_SWIZZLE_ONLY(4, w, z, w, z)
DEFINE_SWIZZLE_ONLY(4, w, z, w, w)
DEFINE_SWIZZLE_ONLY(4, w, w, x, x)
DEFINE_SWIZZLE_ONLY(4, w, w, x, y)
DEFINE_SWIZZLE_ONLY(4, w, w, x, z)
DEFINE_SWIZZLE_ONLY(4, w, w, x, w)
DEFINE_SWIZZLE_ONLY(4, w, w, y, x)
DEFINE_SWIZZLE_ONLY(4, w, w, y, y)
DEFINE_SWIZZLE_ONLY(4, w, w, y, z)
DEFINE_SWIZZLE_ONLY(4, w, w, y, w)
DEFINE_SWIZZLE_ONLY(4, w, w, z, x)
DEFINE_SWIZZLE_ONLY(4, w, w, z, y)
DEFINE_SWIZZLE_ONLY(4, w, w, z, z)
DEFINE_SWIZZLE_ONLY(4, w, w, z, w)
DEFINE_SWIZZLE_ONLY(4, w, w, w, x)
DEFINE_SWIZZLE_ONLY(4, w, w, w, y)
DEFINE_SWIZZLE_ONLY(4, w, w, w, z)
DEFINE_SWIZZLE_ONLY(4, w, w, w, w)

#undef DEFINE_SWIZZLE_ONLY
#undef DEFINE_SWIZZLE_MASK

/******************************************************************************/

template <typename T>
struct vector<T, 1> : __vector_base<T> {
	vector() {
	}

	vector(T x) : x(x) {
	}

	union {
		struct { T x; };
		struct { T r; };
		__xx<T, 1> xx, rr;
		__xxx<T, 1> xxx, rrr;
		__xxxx<T, 1> xxxx, rrrr;
	};
};

/******************************************************************************/

template <typename T>
struct vector<T, 2> : __vector_base<T> {
	vector() {
	}

	vector(T k) : x(k), y(k) {
	}

	vector(T x, T y) : x(x), y(y) {
	}

	union {
		struct { T x, y; };
		struct { T r, g; };

		__xx<T, 2> xx, rr;
		__xy<T, 2> xy, rg;
		__yx<T, 2> yx, gr;
		__yy<T, 2> yy, gg;

		__xxx<T, 2> xxx, rrr;
		__xxy<T, 2> xxy, rrg;
		__xyx<T, 2> xyx, rgr;
		__xyy<T, 2> xyy, rgg;
		__yxx<T, 2> yxx, grr;
		__yxy<T, 2> yxy, grg;
		__yyx<T, 2> yyx, ggr;
		__yyy<T, 2> yyy, ggg;

		__xxxx<T, 2> xxxx, rrrr;
		__xxxy<T, 2> xxxy, rrrg;
		__xxyx<T, 2> xxyx, rrgr;
		__xxyy<T, 2> xxyy, rrgg;
		__xyxx<T, 2> xyxx, rgrr;
		__xyxy<T, 2> xyxy, rgrg;
		__xyyx<T, 2> xyyx, rggr;
		__xyyy<T, 2> xyyy, rggg;
		__yxxx<T, 2> yxxx, grrr;
		__yxxy<T, 2> yxxy, grrg;
		__yxyx<T, 2> yxyx, grgr;
		__yxyy<T, 2> yxyy, grgg;
		__yyxx<T, 2> yyxx, ggrr;
		__yyxy<T, 2> yyxy, ggrg;
		__yyyx<T, 2> yyyx, gggr;
		__yyyy<T, 2> yyyy, gggg;
	};
};

/******************************************************************************/

template <typename T>
struct vector<T, 3> : __vector_base<T> {
	vector() {
	}

	vector(T k) : x(k), y(k), z(k) {
	}

	vector(T x, T y, T z) : x(x), y(y), z(z) {
	}

	vector(const vector<T, 2> & _xy, T z) : x(_xy[0]), y(_xy[1]), z(z) {
	}

	vector(T x, const vector<T, 2> & _yz) : x(x), y(_yz[0]), z(_yz[1]) {
	}

	union {
		struct { T x, y, z; };
		struct { T r, g, b; };

		__xx<T, 3> xx, rr;
		__xy<T, 3> xy, rg;
		__xz<T, 3> xz, rb;
		__yx<T, 3> yx, gr;
		__yy<T, 3> yy, gg;
		__yz<T, 3> yz, gb;
		__zx<T, 3> zx, br;
		__zy<T, 3> zy, bg;
		__zz<T, 3> zz, bb;

		__xxx<T, 3> xxx, rrr;
		__xxy<T, 3> xxy, rrg;
		__xxz<T, 3> xxz, rrb;
		__xyx<T, 3> xyx, rgr;
		__xyy<T, 3> xyy, rgg;
		__xyz<T, 3> xyz, rgb;
		__xzx<T, 3> xzx, rbr;
		__xzy<T, 3> xzy, rbg;
		__xzz<T, 3> xzz, rbb;
		__yxx<T, 3> yxx, grr;
		__yxy<T, 3> yxy, grg;
		__yxz<T, 3> yxz, grb;
		__yyx<T, 3> yyx, ggr;
		__yyy<T, 3> yyy, ggg;
		__yyz<T, 3> yyz, ggb;
		__yzx<T, 3> yzx, gbr;
		__yzy<T, 3> yzy, gbg;
		__yzz<T, 3> yzz, gbb;
		__zxx<T, 3> zxx, brr;
		__zxy<T, 3> zxy, brg;
		__zxz<T, 3> zxz, brb;
		__zyx<T, 3> zyx, bgr;
		__zyy<T, 3> zyy, bgg;
		__zyz<T, 3> zyz, bgb;
		__zzx<T, 3> zzx, bbr;
		__zzy<T, 3> zzy, bbg;
		__zzz<T, 3> zzz, bbb;

		__xxxx<T, 3> xxxx, rrrr;
		__xxxy<T, 3> xxxy, rrrg;
		__xxxz<T, 3> xxxz, rrrb;
		__xxyx<T, 3> xxyx, rrgr;
		__xxyy<T, 3> xxyy, rrgg;
		__xxyz<T, 3> xxyz, rrgb;
		__xxzx<T, 3> xxzx, rrbr;
		__xxzy<T, 3> xxzy, rrbg;
		__xxzz<T, 3> xxzz, rrbb;
		__xyxx<T, 3> xyxx, rgrr;
		__xyxy<T, 3> xyxy, rgrg;
		__xyxz<T, 3> xyxz, rgrb;
		__xyyx<T, 3> xyyx, rggr;
		__xyyy<T, 3> xyyy, rggg;
		__xyyz<T, 3> xyyz, rggb;
		__xyzx<T, 3> xyzx, rgbr;
		__xyzy<T, 3> xyzy, rgbg;
		__xyzz<T, 3> xyzz, rgbb;
		__xzxx<T, 3> xzxx, rbrr;
		__xzxy<T, 3> xzxy, rbrg;
		__xzxz<T, 3> xzxz, rbrb;
		__xzyx<T, 3> xzyx, rbgr;
		__xzyy<T, 3> xzyy, rbgg;
		__xzyz<T, 3> xzyz, rbgb;
		__xzzx<T, 3> xzzx, rbbr;
		__xzzy<T, 3> xzzy, rbbg;
		__xzzz<T, 3> xzzz, rbbb;
		__yxxx<T, 3> yxxx, grrr;
		__yxxy<T, 3> yxxy, grrg;
		__yxxz<T, 3> yxxz, grrb;
		__yxyx<T, 3> yxyx, grgr;
		__yxyy<T, 3> yxyy, grgg;
		__yxyz<T, 3> yxyz, grgb;
		__yxzx<T, 3> yxzx, grbr;
		__yxzy<T, 3> yxzy, grbg;
		__yxzz<T, 3> yxzz, grbb;
		__yyxx<T, 3> yyxx, ggrr;
		__yyxy<T, 3> yyxy, ggrg;
		__yyxz<T, 3> yyxz, ggrb;
		__yyyx<T, 3> yyyx, gggr;
		__yyyy<T, 3> yyyy, gggg;
		__yyyz<T, 3> yyyz, gggb;
		__yyzx<T, 3> yyzx, ggbr;
		__yyzy<T, 3> yyzy, ggbg;
		__yyzz<T, 3> yyzz, ggbb;
		__yzxx<T, 3> yzxx, gbrr;
		__yzxy<T, 3> yzxy, gbrg;
		__yzxz<T, 3> yzxz, gbrb;
		__yzyx<T, 3> yzyx, gbgr;
		__yzyy<T, 3> yzyy, gbgg;
		__yzyz<T, 3> yzyz, gbgb;
		__yzzx<T, 3> yzzx, gbbr;
		__yzzy<T, 3> yzzy, gbbg;
		__yzzz<T, 3> yzzz, gbbb;
		__zxxx<T, 3> zxxx, brrr;
		__zxxy<T, 3> zxxy, brrg;
		__zxxz<T, 3> zxxz, brrb;
		__zxyx<T, 3> zxyx, brgr;
		__zxyy<T, 3> zxyy, brgg;
		__zxyz<T, 3> zxyz, brgb;
		__zxzx<T, 3> zxzx, brbr;
		__zxzy<T, 3> zxzy, brbg;
		__zxzz<T, 3> zxzz, brbb;
		__zyxx<T, 3> zyxx, bgrr;
		__zyxy<T, 3> zyxy, bgrg;
		__zyxz<T, 3> zyxz, bgrb;
		__zyyx<T, 3> zyyx, bggr;
		__zyyy<T, 3> zyyy, bggg;
		__zyyz<T, 3> zyyz, bggb;
		__zyzx<T, 3> zyzx, bgbr;
		__zyzy<T, 3> zyzy, bgbg;
		__zyzz<T, 3> zyzz, bgbb;
		__zzxx<T, 3> zzxx, bbrr;
		__zzxy<T, 3> zzxy, bbrg;
		__zzxz<T, 3> zzxz, bbrb;
		__zzyx<T, 3> zzyx, bbgr;
		__zzyy<T, 3> zzyy, bbgg;
		__zzyz<T, 3> zzyz, bbgb;
		__zzzx<T, 3> zzzx, bbbr;
		__zzzy<T, 3> zzzy, bbbg;
		__zzzz<T, 3> zzzz, bbbb;
	};
};

/******************************************************************************/

template <typename T>
struct vector<T, 4> : __vector_base<T> {
	vector() {
	}

	vector(T k) : x(k), y(k), z(k), w(k) {
	}

	vector(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {
	}

	vector(const vector<T, 2> & _xy, T z, T w) : x(_xy[0]), y(_xy[1]), z(z), w(w) {
	}

	vector(T x, const vector<T, 2> & _yz, T w) : x(x), y(_yz[0]), z(_yz[1]), w(w) {
	}

	vector(T x, T y, const vector<T, 2> & _zw) : x(x), y(y), z(_zw[0]), w(_zw[1]) {
	}

	vector(const vector<T, 2> & _xy, const vector<T, 2> & _zw) : x(_xy[0]), y(_xy[1]), z(_zw[0]), w(_zw[1]) {
	}

	vector(const vector<T, 3> & _xyz, T w) : x(_xyz[0]), y(_xyz[1]), z(_xyz[2]), w(w) {
	}

	vector(T x, const vector<T, 3> & _yzw) : x(x), y(_yzw[0]), z(_yzw[1]), w(_yzw[2]) {
	}


	// A union with 674 members. Maybe I shouldn't teach software engineering.
	union {
		struct { T x, y, z, w; };
		struct { T r, g, b, a; };

		__xx<T, 4> xx, rr;
		__xy<T, 4> xy, rg;
		__xz<T, 4> xz, rb;
		__xw<T, 4> xw, ra;
		__yx<T, 4> yx, gr;
		__yy<T, 4> yy, gg;
		__yz<T, 4> yz, gb;
		__yw<T, 4> yw, ga;
		__zx<T, 4> zx, br;
		__zy<T, 4> zy, bg;
		__zz<T, 4> zz, bb;
		__zw<T, 4> zw, ba;
		__wx<T, 4> wx, ar;
		__wy<T, 4> wy, ag;
		__wz<T, 4> wz, ab;
		__ww<T, 4> ww, aa;

		__xxx<T, 4> xxx, rrr;
		__xxy<T, 4> xxy, rrg;
		__xxz<T, 4> xxz, rrb;
		__xxw<T, 4> xxw, rra;
		__xyx<T, 4> xyx, rgr;
		__xyy<T, 4> xyy, rgg;
		__xyz<T, 4> xyz, rgb;
		__xyw<T, 4> xyw, rga;
		__xzx<T, 4> xzx, rbr;
		__xzy<T, 4> xzy, rbg;
		__xzz<T, 4> xzz, rbb;
		__xzw<T, 4> xzw, rba;
		__xwx<T, 4> xwx, rar;
		__xwy<T, 4> xwy, rag;
		__xwz<T, 4> xwz, rab;
		__xww<T, 4> xww, raa;
		__yxx<T, 4> yxx, grr;
		__yxy<T, 4> yxy, grg;
		__yxz<T, 4> yxz, grb;
		__yxw<T, 4> yxw, gra;
		__yyx<T, 4> yyx, ggr;
		__yyy<T, 4> yyy, ggg;
		__yyz<T, 4> yyz, ggb;
		__yyw<T, 4> yyw, gga;
		__yzx<T, 4> yzx, gbr;
		__yzy<T, 4> yzy, gbg;
		__yzz<T, 4> yzz, gbb;
		__yzw<T, 4> yzw, gba;
		__ywx<T, 4> ywx, gar;
		__ywy<T, 4> ywy, gag;
		__ywz<T, 4> ywz, gab;
		__yww<T, 4> yww, gaa;
		__zxx<T, 4> zxx, brr;
		__zxy<T, 4> zxy, brg;
		__zxz<T, 4> zxz, brb;
		__zxw<T, 4> zxw, bra;
		__zyx<T, 4> zyx, bgr;
		__zyy<T, 4> zyy, bgg;
		__zyz<T, 4> zyz, bgb;
		__zyw<T, 4> zyw, bga;
		__zzx<T, 4> zzx, bbr;
		__zzy<T, 4> zzy, bbg;
		__zzz<T, 4> zzz, bbb;
		__zzw<T, 4> zzw, bba;
		__zwx<T, 4> zwx, bar;
		__zwy<T, 4> zwy, bag;
		__zwz<T, 4> zwz, bab;
		__zww<T, 4> zww, baa;
		__wxx<T, 4> wxx, arr;
		__wxy<T, 4> wxy, arg;
		__wxz<T, 4> wxz, arb;
		__wxw<T, 4> wxw, ara;
		__wyx<T, 4> wyx, agr;
		__wyy<T, 4> wyy, agg;
		__wyz<T, 4> wyz, agb;
		__wyw<T, 4> wyw, aga;
		__wzx<T, 4> wzx, abr;
		__wzy<T, 4> wzy, abg;
		__wzz<T, 4> wzz, abb;
		__wzw<T, 4> wzw, aba;
		__wwx<T, 4> wwx, aar;
		__wwy<T, 4> wwy, aag;
		__wwz<T, 4> wwz, aab;
		__www<T, 4> www, aaa;

		__xxxx<T, 4> xxxx, rrrr;
		__xxxy<T, 4> xxxy, rrrg;
		__xxxz<T, 4> xxxz, rrrb;
		__xxxw<T, 4> xxxw, rrra;
		__xxyx<T, 4> xxyx, rrgr;
		__xxyy<T, 4> xxyy, rrgg;
		__xxyz<T, 4> xxyz, rrgb;
		__xxyw<T, 4> xxyw, rrga;
		__xxzx<T, 4> xxzx, rrbr;
		__xxzy<T, 4> xxzy, rrbg;
		__xxzz<T, 4> xxzz, rrbb;
		__xxzw<T, 4> xxzw, rrba;
		__xxwx<T, 4> xxwx, rrar;
		__xxwy<T, 4> xxwy, rrag;
		__xxwz<T, 4> xxwz, rrab;
		__xxww<T, 4> xxww, rraa;
		__xyxx<T, 4> xyxx, rgrr;
		__xyxy<T, 4> xyxy, rgrg;
		__xyxz<T, 4> xyxz, rgrb;
		__xyxw<T, 4> xyxw, rgra;
		__xyyx<T, 4> xyyx, rggr;
		__xyyy<T, 4> xyyy, rggg;
		__xyyz<T, 4> xyyz, rggb;
		__xyyw<T, 4> xyyw, rgga;
		__xyzx<T, 4> xyzx, rgbr;
		__xyzy<T, 4> xyzy, rgbg;
		__xyzz<T, 4> xyzz, rgbb;
		__xyzw<T, 4> xyzw, rgba;
		__xywx<T, 4> xywx, rgar;
		__xywy<T, 4> xywy, rgag;
		__xywz<T, 4> xywz, rgab;
		__xyww<T, 4> xyww, rgaa;
		__xzxx<T, 4> xzxx, rbrr;
		__xzxy<T, 4> xzxy, rbrg;
		__xzxz<T, 4> xzxz, rbrb;
		__xzxw<T, 4> xzxw, rbra;
		__xzyx<T, 4> xzyx, rbgr;
		__xzyy<T, 4> xzyy, rbgg;
		__xzyz<T, 4> xzyz, rbgb;
		__xzyw<T, 4> xzyw, rbga;
		__xzzx<T, 4> xzzx, rbbr;
		__xzzy<T, 4> xzzy, rbbg;
		__xzzz<T, 4> xzzz, rbbb;
		__xzzw<T, 4> xzzw, rbba;
		__xzwx<T, 4> xzwx, rbar;
		__xzwy<T, 4> xzwy, rbag;
		__xzwz<T, 4> xzwz, rbab;
		__xzww<T, 4> xzww, rbaa;
		__xwxx<T, 4> xwxx, rarr;
		__xwxy<T, 4> xwxy, rarg;
		__xwxz<T, 4> xwxz, rarb;
		__xwxw<T, 4> xwxw, rara;
		__xwyx<T, 4> xwyx, ragr;
		__xwyy<T, 4> xwyy, ragg;
		__xwyz<T, 4> xwyz, ragb;
		__xwyw<T, 4> xwyw, raga;
		__xwzx<T, 4> xwzx, rabr;
		__xwzy<T, 4> xwzy, rabg;
		__xwzz<T, 4> xwzz, rabb;
		__xwzw<T, 4> xwzw, raba;
		__xwwx<T, 4> xwwx, raar;
		__xwwy<T, 4> xwwy, raag;
		__xwwz<T, 4> xwwz, raab;
		__xwww<T, 4> xwww, raaa;
		__yxxx<T, 4> yxxx, grrr;
		__yxxy<T, 4> yxxy, grrg;
		__yxxz<T, 4> yxxz, grrb;
		__yxxw<T, 4> yxxw, grra;
		__yxyx<T, 4> yxyx, grgr;
		__yxyy<T, 4> yxyy, grgg;
		__yxyz<T, 4> yxyz, grgb;
		__yxyw<T, 4> yxyw, grga;
		__yxzx<T, 4> yxzx, grbr;
		__yxzy<T, 4> yxzy, grbg;
		__yxzz<T, 4> yxzz, grbb;
		__yxzw<T, 4> yxzw, grba;
		__yxwx<T, 4> yxwx, grar;
		__yxwy<T, 4> yxwy, grag;
		__yxwz<T, 4> yxwz, grab;
		__yxww<T, 4> yxww, graa;
		__yyxx<T, 4> yyxx, ggrr;
		__yyxy<T, 4> yyxy, ggrg;
		__yyxz<T, 4> yyxz, ggrb;
		__yyxw<T, 4> yyxw, ggra;
		__yyyx<T, 4> yyyx, gggr;
		__yyyy<T, 4> yyyy, gggg;
		__yyyz<T, 4> yyyz, gggb;
		__yyyw<T, 4> yyyw, ggga;
		__yyzx<T, 4> yyzx, ggbr;
		__yyzy<T, 4> yyzy, ggbg;
		__yyzz<T, 4> yyzz, ggbb;
		__yyzw<T, 4> yyzw, ggba;
		__yywx<T, 4> yywx, ggar;
		__yywy<T, 4> yywy, ggag;
		__yywz<T, 4> yywz, ggab;
		__yyww<T, 4> yyww, ggaa;
		__yzxx<T, 4> yzxx, gbrr;
		__yzxy<T, 4> yzxy, gbrg;
		__yzxz<T, 4> yzxz, gbrb;
		__yzxw<T, 4> yzxw, gbra;
		__yzyx<T, 4> yzyx, gbgr;
		__yzyy<T, 4> yzyy, gbgg;
		__yzyz<T, 4> yzyz, gbgb;
		__yzyw<T, 4> yzyw, gbga;
		__yzzx<T, 4> yzzx, gbbr;
		__yzzy<T, 4> yzzy, gbbg;
		__yzzz<T, 4> yzzz, gbbb;
		__yzzw<T, 4> yzzw, gbba;
		__yzwx<T, 4> yzwx, gbar;
		__yzwy<T, 4> yzwy, gbag;
		__yzwz<T, 4> yzwz, gbab;
		__yzww<T, 4> yzww, gbaa;
		__ywxx<T, 4> ywxx, garr;
		__ywxy<T, 4> ywxy, garg;
		__ywxz<T, 4> ywxz, garb;
		__ywxw<T, 4> ywxw, gara;
		__ywyx<T, 4> ywyx, gagr;
		__ywyy<T, 4> ywyy, gagg;
		__ywyz<T, 4> ywyz, gagb;
		__ywyw<T, 4> ywyw, gaga;
		__ywzx<T, 4> ywzx, gabr;
		__ywzy<T, 4> ywzy, gabg;
		__ywzz<T, 4> ywzz, gabb;
		__ywzw<T, 4> ywzw, gaba;
		__ywwx<T, 4> ywwx, gaar;
		__ywwy<T, 4> ywwy, gaag;
		__ywwz<T, 4> ywwz, gaab;
		__ywww<T, 4> ywww, gaaa;
		__zxxx<T, 4> zxxx, brrr;
		__zxxy<T, 4> zxxy, brrg;
		__zxxz<T, 4> zxxz, brrb;
		__zxxw<T, 4> zxxw, brra;
		__zxyx<T, 4> zxyx, brgr;
		__zxyy<T, 4> zxyy, brgg;
		__zxyz<T, 4> zxyz, brgb;
		__zxyw<T, 4> zxyw, brga;
		__zxzx<T, 4> zxzx, brbr;
		__zxzy<T, 4> zxzy, brbg;
		__zxzz<T, 4> zxzz, brbb;
		__zxzw<T, 4> zxzw, brba;
		__zxwx<T, 4> zxwx, brar;
		__zxwy<T, 4> zxwy, brag;
		__zxwz<T, 4> zxwz, brab;
		__zxww<T, 4> zxww, braa;
		__zyxx<T, 4> zyxx, bgrr;
		__zyxy<T, 4> zyxy, bgrg;
		__zyxz<T, 4> zyxz, bgrb;
		__zyxw<T, 4> zyxw, bgra;
		__zyyx<T, 4> zyyx, bggr;
		__zyyy<T, 4> zyyy, bggg;
		__zyyz<T, 4> zyyz, bggb;
		__zyyw<T, 4> zyyw, bgga;
		__zyzx<T, 4> zyzx, bgbr;
		__zyzy<T, 4> zyzy, bgbg;
		__zyzz<T, 4> zyzz, bgbb;
		__zyzw<T, 4> zyzw, bgba;
		__zywx<T, 4> zywx, bgar;
		__zywy<T, 4> zywy, bgag;
		__zywz<T, 4> zywz, bgab;
		__zyww<T, 4> zyww, bgaa;
		__zzxx<T, 4> zzxx, bbrr;
		__zzxy<T, 4> zzxy, bbrg;
		__zzxz<T, 4> zzxz, bbrb;
		__zzxw<T, 4> zzxw, bbra;
		__zzyx<T, 4> zzyx, bbgr;
		__zzyy<T, 4> zzyy, bbgg;
		__zzyz<T, 4> zzyz, bbgb;
		__zzyw<T, 4> zzyw, bbga;
		__zzzx<T, 4> zzzx, bbbr;
		__zzzy<T, 4> zzzy, bbbg;
		__zzzz<T, 4> zzzz, bbbb;
		__zzzw<T, 4> zzzw, bbba;
		__zzwx<T, 4> zzwx, bbar;
		__zzwy<T, 4> zzwy, bbag;
		__zzwz<T, 4> zzwz, bbab;
		__zzww<T, 4> zzww, bbaa;
		__zwxx<T, 4> zwxx, barr;
		__zwxy<T, 4> zwxy, barg;
		__zwxz<T, 4> zwxz, barb;
		__zwxw<T, 4> zwxw, bara;
		__zwyx<T, 4> zwyx, bagr;
		__zwyy<T, 4> zwyy, bagg;
		__zwyz<T, 4> zwyz, bagb;
		__zwyw<T, 4> zwyw, baga;
		__zwzx<T, 4> zwzx, babr;
		__zwzy<T, 4> zwzy, babg;
		__zwzz<T, 4> zwzz, babb;
		__zwzw<T, 4> zwzw, baba;
		__zwwx<T, 4> zwwx, baar;
		__zwwy<T, 4> zwwy, baag;
		__zwwz<T, 4> zwwz, baab;
		__zwww<T, 4> zwww, baaa;
		__wxxx<T, 4> wxxx, arrr;
		__wxxy<T, 4> wxxy, arrg;
		__wxxz<T, 4> wxxz, arrb;
		__wxxw<T, 4> wxxw, arra;
		__wxyx<T, 4> wxyx, argr;
		__wxyy<T, 4> wxyy, argg;
		__wxyz<T, 4> wxyz, argb;
		__wxyw<T, 4> wxyw, arga;
		__wxzx<T, 4> wxzx, arbr;
		__wxzy<T, 4> wxzy, arbg;
		__wxzz<T, 4> wxzz, arbb;
		__wxzw<T, 4> wxzw, arba;
		__wxwx<T, 4> wxwx, arar;
		__wxwy<T, 4> wxwy, arag;
		__wxwz<T, 4> wxwz, arab;
		__wxww<T, 4> wxww, araa;
		__wyxx<T, 4> wyxx, agrr;
		__wyxy<T, 4> wyxy, agrg;
		__wyxz<T, 4> wyxz, agrb;
		__wyxw<T, 4> wyxw, agra;
		__wyyx<T, 4> wyyx, aggr;
		__wyyy<T, 4> wyyy, aggg;
		__wyyz<T, 4> wyyz, aggb;
		__wyyw<T, 4> wyyw, agga;
		__wyzx<T, 4> wyzx, agbr;
		__wyzy<T, 4> wyzy, agbg;
		__wyzz<T, 4> wyzz, agbb;
		__wyzw<T, 4> wyzw, agba;
		__wywx<T, 4> wywx, agar;
		__wywy<T, 4> wywy, agag;
		__wywz<T, 4> wywz, agab;
		__wyww<T, 4> wyww, agaa;
		__wzxx<T, 4> wzxx, abrr;
		__wzxy<T, 4> wzxy, abrg;
		__wzxz<T, 4> wzxz, abrb;
		__wzxw<T, 4> wzxw, abra;
		__wzyx<T, 4> wzyx, abgr;
		__wzyy<T, 4> wzyy, abgg;
		__wzyz<T, 4> wzyz, abgb;
		__wzyw<T, 4> wzyw, abga;
		__wzzx<T, 4> wzzx, abbr;
		__wzzy<T, 4> wzzy, abbg;
		__wzzz<T, 4> wzzz, abbb;
		__wzzw<T, 4> wzzw, abba;
		__wzwx<T, 4> wzwx, abar;
		__wzwy<T, 4> wzwy, abag;
		__wzwz<T, 4> wzwz, abab;
		__wzww<T, 4> wzww, abaa;
		__wwxx<T, 4> wwxx, aarr;
		__wwxy<T, 4> wwxy, aarg;
		__wwxz<T, 4> wwxz, aarb;
		__wwxw<T, 4> wwxw, aara;
		__wwyx<T, 4> wwyx, aagr;
		__wwyy<T, 4> wwyy, aagg;
		__wwyz<T, 4> wwyz, aagb;
		__wwyw<T, 4> wwyw, aaga;
		__wwzx<T, 4> wwzx, aabr;
		__wwzy<T, 4> wwzy, aabg;
		__wwzz<T, 4> wwzz, aabb;
		__wwzw<T, 4> wwzw, aaba;
		__wwwx<T, 4> wwwx, aaar;
		__wwwy<T, 4> wwwy, aaag;
		__wwwz<T, 4> wwwz, aaab;
		__wwww<T, 4> wwww, aaaa;
	};
};

/******************************************************************************/

// TODO: Matrix swizzling and masking

template <typename T, int M, int N>
struct matrix;

// TODO: Should this return a boolMxN??

template <typename T, int M, int N>
bool operator==(const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			if (a[i][j] != b[i][j]) {
				return false;
			}
		}
	}

	return true;
}

template <typename T, int M, int N>
bool operator!=(const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			if (a[i][j] != b[i][j]) {
				return true;
			}
		}
	}

	return false;
}

template <typename T, int M, int N>
matrix<T, M, N> operator-(const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	matrix<T, M, N> c;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			c[i][j] = a[i][j] - b[i][j];
		}
	}
	return c;
}

template <typename T, int M, int N>
matrix<T, M, N> operator+(const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	matrix<T, M, N> c;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			c[i][j] = a[i][j] + b[i][j];
		}
	}
	return c;
}

template <typename T, int M, int N>
matrix<T, M, N> operator*(const matrix<T, M, N> & a, const matrix<T, M, N> & b) {
	matrix<T, M, N> c;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			c[i][j] = a[i][j] * b[i][j];
		}
	}
	return c;
}

template <typename T, int M, int N>
matrix<T, M, N> operator*(const matrix<T, M, N> & a, T b) {
	matrix<T, M, N> c;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			c[i][j] = a[i][j] * b;
		}
	}
	return c;
}

template <typename T, int M, int N>
matrix<T, M, N> operator*(T a, const matrix<T, M, N> & b) {
	matrix<T, M, N> c;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			c[i][j] = a * b[i][j];
		}
	}
	return c;
}

/******************************************************************************/

template <typename T, int N>
struct __vector;

template <typename T>
struct __vector<T, 2> {
	inline __vector() {
	}

	inline __vector(T * x, T * y) : x(x), y(y) {
	}

	inline __vector & operator=(const vector<T, 2> & b) {
		*x = b.x;
		*y = b.y;
		return *this;
	}

	inline T & operator[](int index) {
		T ** p = (T **) this;
		return *p[index];
	}

	inline operator vector<T, 2>() const {
		return vector<T, 2>(*x, *y);
	}

	T * x, * y;
};

template <typename T>
struct __vector<T, 3> {
	inline __vector() {
	}

	inline __vector(T * x, T * y, T * z) : x(x), y(y), z(z) {
	}

	inline __vector & operator=(const vector<T, 3> & b) {
		*x = b.x;
		*y = b.y;
		*z = b.z;
		return *this;
	}

	inline T & operator[](int index) {
		T ** p = (T **) this;
		return *p[index];
	}

	inline operator vector<T, 3>() const {
		return vector<T, 3>(*x, *y, *z);
	}

	T * x, * y, * z;
};

template <typename T>
struct __vector<T, 4> {
	inline __vector() {
	}

	inline __vector(T * x, T * y, T * z, T * w) : x(x), y(y), z(z), w(w) {
	}

	inline __vector & operator=(const vector<T, 4> & b) {
		*x = b.x;
		*y = b.y;
		*z = b.z;
		*w = b.w;
		return *this;
	}

	inline T & operator[](int index) {
		T ** p = (T **) this;
		return *p[index];
	}

	inline operator vector<T, 4>() const {
		return vector<T, 4>(*x, *y, *z, *w);
	}

	T * x, * y, * z, * w;
};

template <typename T, int M, int N>
struct __matrix_base {
	inline operator const T *() const {
		return (const T *) this;
	}

	inline operator T *() const {
		return (T *) this;
	}

	inline vector<T, N>operator[](int i) const {
		const T * a = (const T *) this;
		vector<T, N> b;
		for (int j = 0; j < N ; j++) {
			b[j] = a[i + M * j];
		}
		return b;
	}

	inline __vector<T, N>operator[](int i) {
		T * a = (T *) this;
		__vector<T, N> b;
		T ** p = (T **) &b;
		for (int j = 0; j < N ; j++) {
			p[j] = &a[i + M * j];
		}
		return b;
	}
};

/******************************************************************************/

template <typename T>
struct matrix<T, 1, 1> : __matrix_base<T, 1, 1> {
	matrix() {
	}

	matrix(T _m00) : _m00(_m00) {
	}

	union {
		T _m00;
		T _11;
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 1, 2> : __matrix_base<T, 1, 2> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), 
		_m01(k) {
	}

	matrix(
		T _m00, T _m01) : 
		_m00(_m00), 
		_m01(_m01) {
	}

	union {
		struct {
			T _m00;
			T _m01;
 		};
		struct {
			T _11;
			T _12;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 1, 3> : __matrix_base<T, 1, 3> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), 
		_m01(k), 
		_m02(k) {
	}

	matrix(
		T _m00, T _m01, T _m02) : 
		_m00(_m00), 
		_m01(_m01), 
		_m02(_m02) {
	}

	union {
		struct {
			T _m00;
			T _m01;
			T _m02;
 		};
		struct {
			T _11;
			T _12;
			T _13;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 1, 4> : __matrix_base<T, 1, 4> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), 
		_m01(k), 
		_m02(k), 
		_m03(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, T _m03) : 
		_m00(_m00), 
		_m01(_m01), 
		_m02(_m02), 
		_m03(_m03) {
	}

	union {
		struct {
			T _m00;
			T _m01;
			T _m02;
			T _m03;
 		};
		struct {
			T _11;
			T _12;
			T _13;
			T _14;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 2, 1> : __matrix_base<T, 2, 1> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k) {
	}

	matrix(
		T _m00, 
		T _m10) : 
		_m00(_m00), _m10(_m10) {
	}

	union {
		struct {
			T _m00, _m10;
 		};
		struct {
			T _11, _21;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 2, 2> : __matrix_base<T, 2, 2> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), 
		_m01(k), _m11(k) {
	}

	matrix(
		T _m00, T _m01, 
		T _m10, T _m11) : 
		_m00(_m00), _m10(_m10), 
		_m01(_m01), _m11(_m11) {
	}

	union {
		struct {
			T _m00, _m10;
			T _m01, _m11;
 		};
		struct {
			T _11, _21;
			T _12, _22;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 2, 3> : __matrix_base<T, 2, 3> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), 
		_m01(k), _m11(k), 
		_m02(k), _m12(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, 
		T _m10, T _m11, T _m12) : 
		_m00(_m00), _m10(_m10), 
		_m01(_m01), _m11(_m11), 
		_m02(_m02), _m12(_m12) {
	}

	union {
		struct {
			T _m00, _m10;
			T _m01, _m11;
			T _m02, _m12;
 		};
		struct {
			T _11, _21;
			T _12, _22;
			T _13, _23;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 2, 4> : __matrix_base<T, 2, 4> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), 
		_m01(k), _m11(k), 
		_m02(k), _m12(k), 
		_m03(k), _m13(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, T _m03, 
		T _m10, T _m11, T _m12, T _m13) : 
		_m00(_m00), _m10(_m10), 
		_m01(_m01), _m11(_m11), 
		_m02(_m02), _m12(_m12), 
		_m03(_m03), _m13(_m13) {
	}

	union {
		struct {
			T _m00, _m10;
			T _m01, _m11;
			T _m02, _m12;
			T _m03, _m13;
 		};
		struct {
			T _11, _21;
			T _12, _22;
			T _13, _23;
			T _14, _24;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 3, 1> : __matrix_base<T, 3, 1> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k) {
	}

	matrix(
		T _m00, 
		T _m10, 
		T _m20) : 
		_m00(_m00), _m10(_m10), _m20(_m20) {
	}

	union {
		struct {
			T _m00, _m10, _m20;
 		};
		struct {
			T _11, _21, _31;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 3, 2> : __matrix_base<T, 3, 2> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), 
		_m01(k), _m11(k), _m21(k) {
	}

	matrix(
		T _m00, T _m01, 
		T _m10, T _m11, 
		T _m20, T _m21) : 
		_m00(_m00), _m10(_m10), _m20(_m20), 
		_m01(_m01), _m11(_m11), _m21(_m21) {
	}

	union {
		struct {
			T _m00, _m10, _m20;
			T _m01, _m11, _m21;
 		};
		struct {
			T _11, _21, _31;
			T _12, _22, _32;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 3, 3> : __matrix_base<T, 3, 3> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), 
		_m01(k), _m11(k), _m21(k), 
		_m02(k), _m12(k), _m22(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, 
		T _m10, T _m11, T _m12, 
		T _m20, T _m21, T _m22) : 
		_m00(_m00), _m10(_m10), _m20(_m20), 
		_m01(_m01), _m11(_m11), _m21(_m21), 
		_m02(_m02), _m12(_m12), _m22(_m22) {
	}

	union {
		struct {
			T _m00, _m10, _m20;
			T _m01, _m11, _m21;
			T _m02, _m12, _m22;
 		};
		struct {
			T _11, _21, _31;
			T _12, _22, _32;
			T _13, _23, _33;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 3, 4> : __matrix_base<T, 3, 4> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), 
		_m01(k), _m11(k), _m21(k), 
		_m02(k), _m12(k), _m22(k), 
		_m03(k), _m13(k), _m23(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, T _m03, 
		T _m10, T _m11, T _m12, T _m13, 
		T _m20, T _m21, T _m22, T _m23) : 
		_m00(_m00), _m10(_m10), _m20(_m20), 
		_m01(_m01), _m11(_m11), _m21(_m21), 
		_m02(_m02), _m12(_m12), _m22(_m22), 
		_m03(_m03), _m13(_m13), _m23(_m23) {
	}

	union {
		struct {
			T _m00, _m10, _m20;
			T _m01, _m11, _m21;
			T _m02, _m12, _m22;
			T _m03, _m13, _m23;
 		};
		struct {
			T _11, _21, _31;
			T _12, _22, _32;
			T _13, _23, _33;
			T _14, _24, _34;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 4, 1> : __matrix_base<T, 4, 1> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), _m30(k) {
	}

	matrix(
		T _m00, 
		T _m10, 
		T _m20, 
		T _m30) : 
		_m00(_m00), _m10(_m10), _m20(_m20), _m30(_m30) {
	}

	union {
		struct {
			T _m00, _m10, _m20, _m30;
 		};
		struct {
			T _11, _21, _31, _41;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 4, 2> : __matrix_base<T, 4, 2> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), _m30(k), 
		_m01(k), _m11(k), _m21(k), _m31(k) {
	}

	matrix(
		T _m00, T _m01, 
		T _m10, T _m11, 
		T _m20, T _m21, 
		T _m30, T _m31) : 
		_m00(_m00), _m10(_m10), _m20(_m20), _m30(_m30), 
		_m01(_m01), _m11(_m11), _m21(_m21), _m31(_m31) {
	}

	union {
		struct {
			T _m00, _m10, _m20, _m30;
			T _m01, _m11, _m21, _m31;
 		};
		struct {
			T _11, _21, _31, _41;
			T _12, _22, _32, _42;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 4, 3> : __matrix_base<T, 4, 3> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), _m30(k), 
		_m01(k), _m11(k), _m21(k), _m31(k), 
		_m02(k), _m12(k), _m22(k), _m32(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, 
		T _m10, T _m11, T _m12, 
		T _m20, T _m21, T _m22, 
		T _m30, T _m31, T _m32) : 
		_m00(_m00), _m10(_m10), _m20(_m20), _m30(_m30), 
		_m01(_m01), _m11(_m11), _m21(_m21), _m31(_m31), 
		_m02(_m02), _m12(_m12), _m22(_m22), _m32(_m32) {
	}

	union {
		struct {
			T _m00, _m10, _m20, _m30;
			T _m01, _m11, _m21, _m31;
			T _m02, _m12, _m22, _m32;
 		};
		struct {
			T _11, _21, _31, _41;
			T _12, _22, _32, _42;
			T _13, _23, _33, _43;
 		};
	};
};

/******************************************************************************/

template <typename T>
struct matrix<T, 4, 4> : __matrix_base<T, 4, 4> {
	matrix() {
	}

	matrix(T k) : 
		_m00(k), _m10(k), _m20(k), _m30(k), 
		_m01(k), _m11(k), _m21(k), _m31(k), 
		_m02(k), _m12(k), _m22(k), _m32(k), 
		_m03(k), _m13(k), _m23(k), _m33(k) {
	}

	matrix(
		T _m00, T _m01, T _m02, T _m03, 
		T _m10, T _m11, T _m12, T _m13, 
		T _m20, T _m21, T _m22, T _m23, 
		T _m30, T _m31, T _m32, T _m33) : 
		_m00(_m00), _m10(_m10), _m20(_m20), _m30(_m30), 
		_m01(_m01), _m11(_m11), _m21(_m21), _m31(_m31), 
		_m02(_m02), _m12(_m12), _m22(_m22), _m32(_m32), 
		_m03(_m03), _m13(_m13), _m23(_m23), _m33(_m33) {
	}

	union {
		struct {
			T _m00, _m10, _m20, _m30;
			T _m01, _m11, _m21, _m31;
			T _m02, _m12, _m22, _m32;
			T _m03, _m13, _m23, _m33;
 		};
		struct {
			T _11, _21, _31, _41;
			T _12, _22, _32, _42;
			T _13, _23, _33, _43;
			T _14, _24, _34, _44;
 		};
	};
};

/******************************************************************************/

#ifdef _MSC_VER
typedef unsigned __int32 uint;
#else
typedef unsigned int uint;
#endif

typedef vector<float, 1> float1;
typedef vector<float, 2> float2;
typedef vector<float, 3> float3;
typedef vector<float, 4> float4;

typedef vector<int, 1> int1;
typedef vector<int, 2> int2;
typedef vector<int, 3> int3;
typedef vector<int, 4> int4;

typedef vector<uint, 1> uint1;
typedef vector<uint, 2> uint2;
typedef vector<uint, 3> uint3;
typedef vector<uint, 4> uint4;

typedef matrix<float, 1, 1> float1x1;
typedef matrix<float, 1, 2> float1x2;
typedef matrix<float, 1, 3> float1x3;
typedef matrix<float, 1, 4> float1x4;

typedef matrix<float, 2, 1> float2x1;
typedef matrix<float, 2, 2> float2x2;
typedef matrix<float, 2, 3> float2x3;
typedef matrix<float, 2, 4> float2x4;

typedef matrix<float, 3, 1> float3x1;
typedef matrix<float, 3, 2> float3x2;
typedef matrix<float, 3, 3> float3x3;
typedef matrix<float, 3, 4> float3x4;

typedef matrix<float, 4, 1> float4x1;
typedef matrix<float, 4, 2> float4x2;
typedef matrix<float, 4, 3> float4x3;
typedef matrix<float, 4, 4> float4x4;

/******************************************************************************/
/*                                Intrinsics                                  */
/******************************************************************************/

/* abs */

inline int abs(int x) {
	return x < 0 ? -x : x;
}

inline float abs(float x) {
	return x < 0.0f ? -x : x;
}

//template <typename T>
//T abs(T x) {
//	return x < (T) 0 ? -x : x;
//}

//template <typename T, int N>
//vector<T, N> abs(const vector<T, N> & a) {
//	vector<T, N> b;
//	for (int i = 0; i < N; i++) {
//		b[i] = abs(a[i]);
//	}
//	return b;
//}

#define DEFINE_UNARY(__name__, __type__, __dim__)\
	inline __type__##__dim__ __name__(const __type__##__dim__ & a) {\
	__type__##__dim__ b;\
	for (int i = 0; i < __dim__; i++) {\
		b[i] = __name__(a[i]);\
	}\
	return b;\
}\

DEFINE_UNARY(abs, float, 1)
DEFINE_UNARY(abs, float, 2)
DEFINE_UNARY(abs, float, 3)
DEFINE_UNARY(abs, float, 4)
DEFINE_UNARY(abs, int, 1)
DEFINE_UNARY(abs, int, 2)
DEFINE_UNARY(abs, int, 3)
DEFINE_UNARY(abs, int, 4)

#undef DEFINE_UNARY

template <typename T, int M, int N>
matrix<T, M, N> abs(const matrix<T, M, N> & a) {
	matrix<T, M, N> b;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < M; j++) {
			b[i][j] = abs((T) a[i][j]);
		}
	}
	return b;
}

/* acos */

#ifdef _MSC_VER
float acos(float x);
#else
inline float acos(float x) {
	return std::acos(x);
}
#endif

/* all */

template <typename T, int N>
bool all(const vector<T, N> & a) {
	for (int i = 0; i < N; i++) {
		if (!a[i]) {
			return false;
		}
	}
	return true;
}

/* any */

template <typename T, int N>
bool any(const vector<T, N> & a) {
	for (int i = 0; i < N; i++) {
		if (a[i]) {
			return true;
		}
	}
	return false;
}

/* asfloat */

inline float asfloat(uint x) {
	return *(float *) &x;
}

inline float asfloat(int x) {
	return *(float *) &x;
}

/* asin */

#ifdef _MSC_VER
float asin(float x);
#else
inline float asin(float x) {
	return std::asin(x);
}
#endif

/* asint */

inline int asint(float x) {
	return *(int *) &x;
}

inline int asint(uint x) {
	return *(int *) &x;
}

/* asuint */

inline uint asuint(float x) {
	return *(uint *) &x;
}

inline uint asuint(int x) {
	return *(uint *) &x;
}

#ifdef _MSC_VER
float atan2(float y, float x);
#else
inline float atan2(float y, float x) {
	return std::atan2(y, x);
}
#endif

/* clamp */

template <typename T>
inline T clamp(T x, T a, T b) {
	return x < a ? a : x > b ? b : x;
}

/* cos */

#ifdef _MSC_VER
float cos(float x);
#else
inline float cos(float x) {
	return std::cos(x);
}
#endif

/* countbits */

inline uint countbits(uint x){
	uint n = 0;
	for (int i = 0; i < 32; i++) {
		n += x & 1;
		x >>= 1;
	}
	return n;
}

/* cross */

template <typename T>
vector<T, 3> cross(const vector<T, 3> & a, const vector<T, 3> & b) {
	vector<T, 3> c;
	c[0] = a[1] * b[2] - a[2] * b[1];
	c[1] = a[2] * b[0] - a[0] * b[2];
	c[2] = a[0] * b[1] - a[1] * b[0];
	return c;
}

/* ########################################################################## */
/* # Clean up below!                                                        # */
/* ########################################################################## */

/* tan */

#ifdef _MSC_VER
float tan(float x);
#else
inline float tan(float x) {
	return std::tan(x);
}
#endif


/* sin */

#ifdef _MSC_VER
float sin(float x);
#else
inline float sin(float x) {
	return std::sin(x);
}
#endif

/* sqrt */

#ifdef _MSC_VER
float sqrt(float x);
#else
inline float sqrt(float x) {
	return std::sqrt(x);
}
#endif

/* firstbitlow */

inline int firstbitlow(int x){
	for (int i = 0; i < 32; i++) {
		if (x & 1)
			return i;
		x >>= 1;
	}
	return -1;
}

/* firstbithigh */

inline int firstbithigh(int x){
	for (int i = 31; i >= 0; i--) {
		if (x & 0x80000000)
			return i;
		x <<= 1;
	}
	return -1;
}


/* degrees */

inline float degrees(float x) {
	return x * 57.295779513082320876798154814105f;
}

/* determinant */

template <typename T>
T determinant(const matrix<T, 2, 2> & a) {
	return a._m00 * a._m11 - a._m01 * a._m10;
}

template <typename T>
T determinant(const matrix<T, 3, 3> & a) {
	return 
		a._m00 * (a._m11 * a._m22 - a._m21 * a._m12) + 
		a._m10 * (a._m02 * a._m21 - a._m22 * a._m01) + 
		a._m20 * (a._m01 * a._m12 - a._m11 * a._m02);
}

template <typename T>
T determinant(const matrix<T, 4, 4> & a) {
	return
		a._m00 * ((a._m22 * a._m33 * a._m11 + a._m32 * a._m13 * a._m21 + a._m12 * a._m23 * a._m31) - (a._m32 * a._m23 * a._m11 + a._m12 * a._m33 * a._m21 + a._m22 * a._m13 * a._m31)) + 
		a._m10 * ((a._m32 * a._m23 * a._m01 + a._m02 * a._m33 * a._m21 + a._m22 * a._m03 * a._m31) - (a._m22 * a._m33 * a._m01 + a._m32 * a._m03 * a._m21 + a._m02 * a._m23 * a._m31)) + 
		a._m20 * ((a._m12 * a._m33 * a._m01 + a._m32 * a._m03 * a._m11 + a._m02 * a._m13 * a._m31) - (a._m32 * a._m13 * a._m01 + a._m02 * a._m33 * a._m11 + a._m12 * a._m03 * a._m31)) + 
		a._m30 * ((a._m22 * a._m13 * a._m01 + a._m02 * a._m23 * a._m11 + a._m12 * a._m03 * a._m21) - (a._m12 * a._m23 * a._m01 + a._m22 * a._m03 * a._m11 + a._m02 * a._m13 * a._m21));
}

/* distance */

template <typename T, int N>
inline T distance(const vector<T, N> & a, const vector<T, N> & b) {
	return length(a - b);
}

/* dot */

template <typename T, int N>
T dot(const vector<T, N> & a, const vector<T, N> & b) {
	T d = (T) 0;
	for (int i = 0; i < N; i++) {
		d += a[i] * b[i];
	}
	return d;
}

/* length */

template <typename T, int N>
T length(const vector<T, N> & a) {
	return sqrt(dot(a, a));
}

/* lerp */

template <typename T>
T lerp(T a, T b, T t) {
#ifdef HLSL_STABLE_SWIZZLE
	// THIS IS MORE STABLE
	return a * ((T) 1 - t) + b * t;
#else
	// THIS IS FASTER
	return a + (b - a) * t;
#endif
}

template <typename T, int N>
vector<T, N> lerp(const vector<T, N> & a, const vector<T, N> & b, const vector<T, N> & t) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = lerp(a[i], b[i], t[i]);
	}
	return c;
}

template <typename T, int N>
vector<T, N> lerp(const vector<T, N> & a, const vector<T, N> & b, T t) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = lerp(a[i], b[i], t);
	}
	return c;
}

/* max */

template <typename T>
inline T max(T a, T b) {
	return a > b ? a : b;
}

template <typename T, int N>
vector<T, N> max(const vector<T, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = max(a[i], b[i]);
	}
	return c;
}

/* min */

template <typename T>
inline T min(T a, T b) {
	return a < b ? a : b;
}

template <typename T, int N>
vector<T, N> min(const vector<T, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < N; i++) {
		c[i] = min(a[i], b[i]);
	}
	return c;
}

/* mul */

template <typename T, int M, int N, int O>
matrix<T, M, N> mul(const matrix<T, M, O> & a, const matrix<T, O, N> & b) {
	matrix<T, M, N> c;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			c[i][j] = (T) 0;
			for (int k = 0; k < O; k++) {
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}
	return c;
}

template <typename T, int M, int N>
vector<T, N> mul(const vector<T, M> & a, const matrix<T, M, N> & b) {
	vector<T, N> c;
	for (int j = 0; j < N; j++) {
		c[j] = (T) 0;
		for (int i = 0; i < M; i++) {
			c[j] += a[i] * b[i][j];
		}
	}
	return c;
}

template <typename T, int M, int N>
vector<T, M> mul(const matrix<T, M, N> & a, const vector<T, N> & b) {
	vector<T, N> c;
	for (int i = 0; i < M; i++) {
		c[i] = (T) 0;
		for (int j = 0; j < N; j++) {
			c[i] += a[i][j] * b[j];
		}
	}
	return c;
}

/* normalize */

template <typename T, int N>
vector<T, N> normalize(const vector<T, N> & a) {
	return a / length(a);
}

/* radians */

static inline float radians(float x) {
	return x * 0.01745329251994329576923690768489f;
}

/* reflect */

template <typename T, int N>
vector<T, N> reflect(const vector<T, N> & i, const vector<T, N> & n) {
	return i - (T) 2 * n * dot(i, n);
}

/* refract */

template <typename T, int N>
vector<T, N> refract(const vector<T, N> & i, const vector<T, N> & n, T eta) {
	T w = eta * dot(i, n);
	T k = (T) 1 + (w - eta) * (w + eta);
	if (k < (T) 0) {
		// Total internal reflection
		return (T) 0;
	}
	k = sqrt(k);

	return eta * i - (w + k) * n;
}

/* saturate */

template <typename T>
inline T saturate(T x) {
	return clamp(x, (T) 0, (T) 1);
}


template <typename T>
inline T sign(T x){
	if (x > (T)0)
		return (T) 1;
	if (x == (T)0)
		return (T) 0;
	else
		return (T)-1;
}

template <typename T, int N>
vector<T, N> sign(const vector<T, N> & a) {
	vector<T, N> ret;
	for (int i = 0; i < N; i++) {
		ret[i] = sign(a[i]);
	}
	return ret;
}

/* transpose */

template <typename T, int M, int N>
matrix<T, N, M> transpose(const matrix<T, M, N> & a) {
	matrix<T, N, M> b;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			b[j][i] = a[i][j];
		}
	}
	return b;
}

/******************************************************************************/

}

#ifdef _MSC_VER
#pragma warning(default: 4201)
#elif __GNUG__
#pragma GCC diagnostic pop
#endif
#endif
