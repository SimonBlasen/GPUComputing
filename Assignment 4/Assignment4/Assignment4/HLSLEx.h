
#ifndef _HLSL_EX_H_
#define _HLSL_EX_H_

#include "HLSL.h"

namespace hlsl {

/******************************************************************************/
/*                                   Types                                    */
/******************************************************************************/

template <typename T>
struct quaternion {
	quaternion() {
	}

	quaternion(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {
	}

	T x, y, z, w;
};

template <typename T>
bool operator==(const quaternion<T> & a, const quaternion<T> & b) {
	return a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w;
}

template <typename T>
bool operator!=(const quaternion<T> & a, const quaternion<T> & b) {
	return a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w;
}

template <typename T>
quaternion<T> operator+(const quaternion<T> & a, const quaternion<T> & b) {
	return quaternion<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template <typename T>
quaternion<T> operator-(const quaternion<T> & a, const quaternion<T> & b) {
	return quaternion<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template <typename T>
quaternion<T> operator*(T a, const quaternion<T> & b) {
	return quaternion<T>(a * b.x, a * b.y, a * b.z, a * b.w);
}

template <typename T>
quaternion<T> operator*(const quaternion<T> & a, const quaternion<T> & b) {
	quaternion<T> c;

	c.x = a.w * b.x + b.w * a.x + a.y * b.z - a.z * b.y;
	c.y = a.w * b.y + b.w * a.y + a.z * b.x - a.x * b.z;
	c.z = a.w * b.z + b.w * a.z + a.x * b.y - a.y * b.x;
	c.w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;

	return c;
}

/******************************************************************************/

#ifdef _MSC_VER
typedef unsigned __int8	 uchar;
#else
typedef unsigned char	 uchar;
#endif

typedef quaternion<float> floatq;

/******************************************************************************/
/*                                   Macros                                   */
/******************************************************************************/

#define HLSL_EX_PI		3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825f

/******************************************************************************/
/*                                 Functions                                  */
/******************************************************************************/

template <typename T, int M, int N>
const matrix<T, N, M> adjoint(const matrix<T, M, N> & a) {
	matrix<T, N, M> b;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			int s = 1 - 2 * ((i ^ j) & 1);
			b[j][i] = (T) s * cofactor(a, i, j);
		}
	}
	return b;
}

template <typename T, int M, int N>
const matrix<T, M, N> identity() {
	matrix<T, M, N> a;
	for (int i = 0; i < M; i++) {
		for (int j = 0; j < N; j++) {
			a[i][j] = i == j ? (T) 1 : (T) 0;
		}
	}
	return a;
}

template <typename T, int M, int N>
matrix<T, M, N> asmatrix(const /* unit */ quaternion<T> & a) {
	T _xx = a.x * a.x;
	T _xy = a.x * a.y;
	T _xz = a.x * a.z;
	T _yy = a.y * a.y;
	T _yz = a.y * a.z;
	T _zz = a.z * a.z;
	T _wx = a.w * a.x;
	T _wy = a.w * a.y;
	T _wz = a.w * a.z;

	matrix<T, M, N> b = identity<T, M, N>();

	b._m00 = (T) 1 - (T) 2 * (_yy + _zz);
	b._m01 = (T) 2 * (_xy - _wz);
	b._m02 = (T) 2 * (_xz + _wy);

	b._m10 = (T) 2 * (_xy + _wz);
	b._m11 = (T) 1 - (T) 2 * (_xx + _zz);
	b._m12 = (T) 2 * (_yz - _wx);

	b._m20 = (T) 2 * (_xz - _wy);
	b._m21 = (T) 2 * (_yz + _wx);
	b._m22 = (T) 1 - (T) 2 * (_xx + _yy);

	return b;
}

template <typename T, int M, int N>
T cofactor(const matrix<T, M, N> & a, int i, int j) {
	matrix<T, M - 1, N - 1> b;

	for (int jj = 0; jj < j; jj++) {
		for (int ii = 0; ii < i; ii++) {
			b[ii][jj] = a[ii][jj];
		}
	}

	for (int jj = j + 1; jj < N; jj++) {
		for (int ii = 0; ii < i; ii++) {
			b[ii][jj - 1] = a[ii][jj];
		}
	}

	for (int jj = 0; jj < j; jj++) {
		for (int ii = i + 1; ii < M; ii++) {
			b[ii - 1][jj] = a[ii][jj];
		}
	}

	for (int jj = j + 1; jj < N; jj++) {
		for (int ii = i + 1; ii < M; ii++) {
			b[ii - 1][jj - 1] = a[ii][jj];
		}
	}

	return determinant(b);
}

template <typename T>
T cofactor(const matrix<T, 2, 2> & a, int i, int j) {
	return a[1 - i][1 - j];
}

template <typename T>
quaternion<T> conjugate(const quaternion<T> & a) {
	return quaternion<T>(-a.x, -a.y, -a.z, a.w);
}

template <typename T>
vector<T, 2> cross(const vector<T, 2> & a) {
	return vector<T, 2>(-a.y, a.x);
}

template <typename T>
vector<T, 4> cross(const vector<T, 4> & a, const vector<T, 4> & b, const vector<T, 4> & c) {
	T d0 = (b.z * c.w) - (b.w * c.z);
	T d1 = (b.y * c.w) - (b.w * c.y);
	T d2 = (b.y * c.z) - (b.z * c.y);
	T d3 = (b.x * c.w) - (b.w * c.x);
	T d4 = (b.x * c.z) - (b.z * c.x);
	T d5 = (b.x * c.y) - (b.y * c.x);

	return vector<T, 4>(
		-a.y * d0 + a.z * d1 - a.w * d2, 
		+a.x * d0 - a.z * d3 + a.w * d4, 
		-a.x * d1 + a.y * d3 - a.w * d5, 
		+a.x * d2 - a.y * d4 + a.z * d5);
}

template <typename T, int N>
inline T distance_sqr(const vector<T, N> & a, const vector<T, N> & b) {
	return length_sqr(a - b);
}

template <typename T>
T dot(const quaternion<T> & a, const quaternion<T> & b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template <typename T>
matrix<T, 4, 4> frustum(T x0, T x1, T y0, T y1, T z0, T z1) {
	return matrix<T, 4, 4>(
		(T) 2 * z0 / (x1 - x0),                  (T) 0,                (T) 0, (T) 0, 
		                 (T) 0, (T) 2 * z0 / (y1 - y0),                (T) 0, (T) 0, 
		 (x0 + x1) / (x0 - x1),  (y0 + y1) / (y0 - y1),       z1 / (z1 - z0), (T) 1, 
		                 (T) 0,                  (T) 0, -z1 * z0 / (z1 - z0), (T) 0
	);
}

template <typename T>
matrix<T, 4, 4> frustumGL(T x0, T x1, T y0, T y1, T z0, T z1) {
	return matrix<T, 4, 4>(
		(T) 2 * z0 / (x1 - x0),                  (T) 0,                (T) 0, (T) 0, 
		(T) 0, (T) 2 * z0 / (y1 - y0),                (T) 0, (T) 0, 
		(x0 + x1) / (x0 - x1),  (y0 + y1) / (y0 - y1),       (z1 + z0) / (z1 - z0), (T) 1, 
		(T) 0,                  (T) 0, -2.0f * z1 * z0 / (z1 - z0), (T) 0
		);
}

template <typename T>
inline const matrix<T, 4, 4> invert(const matrix<T, 4, 4> & a) {
	T _zzww = a._m22 * a._m33;
	T _wzzw = a._m32 * a._m23;
	T _yzww = a._m12 * a._m33;
	T _wzyw = a._m32 * a._m13;
	T _yzzw = a._m12 * a._m23;
	T _zzyw = a._m22 * a._m13;
	T _xzww = a._m02 * a._m33;
	T _wzxw = a._m32 * a._m03;
	T _xzzw = a._m02 * a._m23;
	T _zzxw = a._m22 * a._m03;
	T _xzyw = a._m02 * a._m13;
	T _yzxw = a._m12 * a._m03;

	T _zxwy = a._m20 * a._m31;
	T _wxzy = a._m30 * a._m21;
	T _yxwy = a._m10 * a._m31;
	T _wxyy = a._m30 * a._m11;
	T _yxzy = a._m10 * a._m21;
	T _zxyy = a._m20 * a._m11;
	T _xxwy = a._m00 * a._m31;
	T _wxxy = a._m30 * a._m01;
	T _xxzy = a._m00 * a._m21;
	T _zxxy = a._m20 * a._m01;
	T _xxyy = a._m00 * a._m11;
	T _yxxy = a._m10 * a._m01;

	matrix<T, 4, 4> b;

	b._m00 = ((_zzww * a._m11 + _wzyw * a._m21 + _yzzw * a._m31) - (_wzzw * a._m11 + _yzww * a._m21 + _zzyw * a._m31));
	b._m01 = ((_wzzw * a._m01 + _xzww * a._m21 + _zzxw * a._m31) - (_zzww * a._m01 + _wzxw * a._m21 + _xzzw * a._m31));
	b._m02 = ((_yzww * a._m01 + _wzxw * a._m11 + _xzyw * a._m31) - (_wzyw * a._m01 + _xzww * a._m11 + _yzxw * a._m31));
	b._m03 = ((_zzyw * a._m01 + _xzzw * a._m11 + _yzxw * a._m21) - (_yzzw * a._m01 + _zzxw * a._m11 + _xzyw * a._m21));
	b._m10 = ((_wzzw * a._m10 + _yzww * a._m20 + _zzyw * a._m30) - (_zzww * a._m10 + _wzyw * a._m20 + _yzzw * a._m30));
	b._m11 = ((_zzww * a._m00 + _wzxw * a._m20 + _xzzw * a._m30) - (_wzzw * a._m00 + _xzww * a._m20 + _zzxw * a._m30));
	b._m12 = ((_wzyw * a._m00 + _xzww * a._m10 + _yzxw * a._m30) - (_yzww * a._m00 + _wzxw * a._m10 + _xzyw * a._m30));
	b._m13 = ((_yzzw * a._m00 + _zzxw * a._m10 + _xzyw * a._m20) - (_zzyw * a._m00 + _xzzw * a._m10 + _yzxw * a._m20));
	b._m20 = ((_zxwy * a._m13 + _wxyy * a._m23 + _yxzy * a._m33) - (_wxzy * a._m13 + _yxwy * a._m23 + _zxyy * a._m33));
	b._m21 = ((_wxzy * a._m03 + _xxwy * a._m23 + _zxxy * a._m33) - (_zxwy * a._m03 + _wxxy * a._m23 + _xxzy * a._m33));
	b._m22 = ((_yxwy * a._m03 + _wxxy * a._m13 + _xxyy * a._m33) - (_wxyy * a._m03 + _xxwy * a._m13 + _yxxy * a._m33));
	b._m23 = ((_zxyy * a._m03 + _xxzy * a._m13 + _yxxy * a._m23) - (_yxzy * a._m03 + _zxxy * a._m13 + _xxyy * a._m23));
	b._m30 = ((_yxwy * a._m22 + _zxyy * a._m32 + _wxzy * a._m12) - (_yxzy * a._m32 + _zxwy * a._m12 + _wxyy * a._m22));
	b._m31 = ((_xxzy * a._m32 + _zxwy * a._m02 + _wxxy * a._m22) - (_xxwy * a._m22 + _zxxy * a._m32 + _wxzy * a._m02));
	b._m32 = ((_xxwy * a._m12 + _yxxy * a._m32 + _wxyy * a._m02) - (_xxyy * a._m32 + _yxwy * a._m02 + _wxxy * a._m12));
	b._m33 = ((_xxyy * a._m22 + _yxzy * a._m02 + _zxxy * a._m12) - (_xxzy * a._m12 + _yxxy * a._m22 + _zxyy * a._m02));

	T det = a._m00 * b._m00 + a._m10 * b._m01 + a._m20 * b._m02 + a._m30 * b._m03;

	b._m00 /= det;
	b._m01 /= det;
	b._m02 /= det;
	b._m03 /= det;
	b._m10 /= det;
	b._m11 /= det;
	b._m12 /= det;
	b._m13 /= det;
	b._m20 /= det;
	b._m21 /= det;
	b._m22 /= det;
	b._m23 /= det;
	b._m30 /= det;
	b._m31 /= det;
	b._m32 /= det;
	b._m33 /= det;

	return b;
}

template <typename T, int N>
inline const matrix<T, N, N> invert(const matrix<T, N, N> & a) {
	return ((T) 1 / determinant(a)) * adjoint(a);
}

template <typename T>
quaternion<T> invert(const quaternion<T> & a) {
	T s = (T) 1 / length_sqr(a);
	return quaternion<T>(-s * a.x, -s * a.y, -s * a.z, s * a.w);
}

template <typename T, int N>
T length_sqr(const vector<T, N> & a) {
	return dot(a, a);
}

template <typename T>
T length_sqr(const quaternion<T> & a) {
	return dot(a, a);
}

template <typename T>
T length(const quaternion<T> & a) {
	return sqrt(length_sqr(a));
}

template <typename T>
quaternion<T> normalize(const quaternion<T> & a) {
	T s = (T) 1 / length(a);
	return quaternion<T>(a.x *s, a.y * s, a.z * s, a.w * s);
}

template <typename T>
matrix<T, 4, 4> look_at(const vector<T, 3> & eye, const vector<T, 3> & at, const vector<T, 3> & up) {
	vector<T, 3> zaxis = normalize(at - eye);
	vector<T, 3> xaxis = normalize(cross(up, zaxis));
	vector<T, 3> yaxis = cross(zaxis, xaxis);

	return matrix<T, 4, 4>(
		 xaxis.x,          yaxis.x,          zaxis.x,         (T) 0,
		 xaxis.y,          yaxis.y,          zaxis.y,         (T) 0,
		 xaxis.z,          yaxis.z,          zaxis.z,         (T) 0,
		-dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), (T) 1);
}

template <typename T>
vector<T, 3> mul(const vector<T, 3> & a, const quaternion<T> & b) {
	quaternion<T> q = invert(b) * quaternion<T>(a.x, a.y, a.z, 0.0f) * b;
	return vector<T, 3>(q.x, q.y, q.z);
}

template <typename T>
vector<T, 3> mul_unit(const vector<T, 3> & a, const /* unit */ quaternion<T> & b) {
	quaternion<T> q = conjugate(b) * quaternion<T>(a.x, a.y, a.z, 0.0f) * b;
	return vector<T, 3>(q.x, q.y, q.z);
}

template <typename T>
matrix<T, 4, 4> perspective(T fovy, T aspect, T z0, T z1) {
	T y1 = z0 * tan(fovy / (T) 2);
	T y0 = -y1;
	T x1 = aspect * y1;
	T x0 = -x1;
	return frustum<T>(x0, x1, y0, y1, z0, z1);
}

template <typename T>
matrix<T, 4, 4> perspectiveGL(T fovy, T aspect, T z0, T z1) {
	T y1 = z0 * tan(fovy / (T) 2);
	T y0 = -y1;
	T x1 = aspect * y1;
	T x0 = -x1;
	return frustumGL<T>(x0, x1, y0, y1, z0, z1);
}

template<typename T>
matrix<T, 4, 4> ortho(T x0, T x1, T y0, T y1, T z0, T z1) {
	return matrix<T, 4, 4>(
		(T) 2 / (x1 - x0),     (T) 0,                 (T) 0,             (T) 0,
		(T) 0,                 (T) 2 / (y1 - y0),     (T) 0,             (T) 0,
		(T) 0,                 (T) 0,                 (T) 1 / (z1 - z0), (T) 0,
		(x0 + x1) / (x0 - x1), (y1 + y0) / (y0 - y1), z0 / (z0 - z1),    (T) 1);
}

template <typename T>
quaternion<T> rotation_q(const /* unit */ vector<T, 3> & axis, T angle) {
	T ca = cos(angle / (T) 2);
	T sa = sin(angle / (T) 2);

	return quaternion<T>(sa * axis.x, sa * axis.y, sa * axis.z, ca);
}

template <typename T, int M, int N>
matrix<T, M, N> rotation(const /* unit */ vector<T, 3> & axis, T angle) {
	return asmatrix<T, M, N>(rotation_q(axis, angle));
}

template <typename T, int M, int N>
matrix<T, M, N> rotation_x(T angle) {
	T ca = cos(angle);
	T sa = sin(angle);

	matrix<T, M, N> a = identity<T, M, N>();
	a._m11 = +ca;
	a._m12 = +sa;
	a._m21 = -sa;
	a._m22 = +ca;
	return a;
}

template <typename T, int M, int N>
matrix<T, M, N> rotation_y(T angle) {
	T ca = cos(angle);
	T sa = sin(angle);

	matrix<T, M, N> a = identity<T, M, N>();
	a._m00 = +ca;
	a._m02 = -sa;
	a._m20 = +sa;
	a._m22 = +ca;
	return a;
}

template <typename T, int M, int N>
matrix<T, M, N> rotation_z(T angle) {
	T ca = cos(angle);
	T sa = sin(angle);

	matrix<T, M, N> a = identity<T, M, N>();
	a._m00 = +ca;
	a._m01 = +sa;
	a._m10 = -sa;
	a._m11 = +ca;
	return a;
}

template <typename T, int M, int N>
matrix<T, M, N> scale(const vector<T, 3> & s) {
	matrix<T, M, N> a = identity<T, M, N>();
	for (int i = 0; i < 3; i++) {
		a[i][i] = s[i];
	}
	return a;
}

template <typename T>
quaternion<T> slerp(const quaternion<T> & a, const quaternion<T> & b, T t) {
	T theta = acos(dot(a, b));
	T t0 = sin(((T) 1 - t) * theta);
	T t1 = sin(         t  * theta);
	T s = (T) 1 / sin(theta);

	return s * (t0 * a + t1 * b);
}

template <typename T>
inline T sqr(T a) {
	return a * a;
}

template <typename T, int M, int N>
matrix<T, M, N> translation(const vector<T, 3> & t) {
	matrix<T, M, N> a = identity<T, M, N>();
	for (int j = 0; j < 3; j++) {
		a[3][j] = t[j];
	}
	return a;
}

/******************************************************************************/

}

#endif
