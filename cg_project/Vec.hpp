#pragma once
#include <iostream>
#include <vector>
#include <numbers>
#include <cmath>

using std::numbers::pi;
using ld = float;
const int eps = 1e-6;
inline int sgn(ld x) { return (x > -eps) - (x < eps); }

class vec3 {
public:
	ld x, y, z;
	vec3(ld _x, ld _y, ld _z) :x(_x), y(_y), z(_z) {};
	vec3(ld* arr) :x(arr[0]), y(arr[1]), z(arr[2]) {};
	vec3(const ld* arr) :x(arr[0]), y(arr[1]), z(arr[2]) {};
	vec3(float v) : x(v), y(v), z(v) {};
	vec3() : x(0), y(0), z(0) {};

	operator const float* () const {
		float* A = new float[3];
		A[0] = x;
		A[1] = y;
		A[2] = z;
		return A;
	}

	vec3 operator + (vec3 a) const { return vec3(x + a.x, y + a.y, z + a.z); }
	inline void operator += (vec3 a) {
		x += a.x, y += a.y, z += a.z;
	}
	vec3 operator * (ld a) const { return vec3(x * a, y * a, z * a); }

	inline void operator *= (ld c) {
		x *= c, y *= c, z *= c;
	}
	vec3 operator - (vec3 a) const { return vec3(x - a.x, y - a.y, z - a.z); }
	inline void operator -= (vec3 a) {
		x -= a.x, y -= a.y, z -= a.z;
	}
	vec3 operator / (ld a) { return vec3(x / a, y / a, z / a); }
	vec3 operator ^ (vec3 a) { return vec3(y * a.z - z * a.y, -(x * a.z - z * a.x), x * a.y - y * a.x); }
	ld& operator [](int i) {
		if (i == 0) return x;
		if (i == 1) return y;
		if (i == 2) return z;
		return z;
	}
	ld operator * (vec3 a) const { return (x * a.x + y * a.y + z * a.z); }

	inline vec3 operator-() const {
		return vec3(-x, -y, -z);
	}
	friend vec3 operator / (ld a, vec3 v) { return vec3(a / v.x, a / v.y, a / v.z); };
	friend vec3 operator * (ld a, vec3 v) { return vec3(v.x * a, v.y * a, v.z * a); }
	friend vec3 ori(vec3 a, vec3 b, vec3 c) {
		return (b - a) ^ (c - a);
	}
	

	friend ld abs2(vec3 a) { return a.x * a.x + a.y * a.y + a.z * a.z; }
	friend ld abs(vec3 a) { return sqrt(abs2(a)); }
	friend vec3 uni(vec3 a) {
		ld l = abs(a);
		if (l < eps) return vec3();
		return vec3(a.x / l, a.y / l, a.z / l);
	}

	friend ld angle(vec3 a, vec3 b) {
		if (sgn(abs(a)) == 0 || sgn(abs(b)) == 0) return 0;
		return acos((a * b) / abs(a) / abs(b));
	}
	friend vec3 Rx(vec3 v, ld t) {
		ld c = cos(t);
		ld s = sin(t);
		auto [x, y, z] = v;
		ld ny = c * y - s * z;
		ld nz = s * y + c * z;
		return vec3(x, ny, nz);
	}
	friend vec3 Ry(vec3 v, ld t) {
		ld c = cos(t);
		ld s = sin(t);
		auto [x, y, z] = v;
		ld nz = c * z - s * x;
		ld nx = s * z + c * x;
		return vec3(nx, y, nz);
	}
	friend vec3 Rz(vec3 v, ld t) {
		ld c = cos(t);
		ld s = sin(t);
		auto [x, y, z] = v;
		ld nx = c * x - s * y;
		ld ny = s * x + c * y;
		return vec3(nx, ny, z);
	}
	friend vec3 rotate(vec3 v, vec3 axis, ld ang) {
		auto [x, y, z] = axis;
		ld _c = 1 - cos(ang);
		ld c = cos(ang);
		ld s = sin(ang);

		vec3 v0 = { x * x * _c + c    , x * y * _c + z * s ,x * z * _c - y * s };
		vec3 v1 = { x * y * _c - z * s, y * y * _c + c     ,y * z * _c + x * s };
		vec3 v2 = { x * z * _c + y * s, y * z * _c - x * s ,z * z * _c + c };
		vec3 ret = v0 * v.x + v1 * v.y + v2 * v.z;

		return ret;
	}
	bool near_zero() const {
		auto s = 1e-8;
		return (std::fabs(x) < s) && (std::fabs(y) < s) && (std::fabs(z) < s);
	}

	friend vec3 mul(vec3 a, vec3 b) {
		return vec3(a.x * b.x, a.y * b.y, a.z * b.z);
	}

};


class vec4 {
public:
	ld x, y, z, w;
	vec4(ld _x, ld _y, ld _z, ld _w) :x(_x), y(_y), z(_z), w(_w) {};
	vec4() : x(0), y(0), z(0), w(1) {};
	vec4(ld* arr) :x(arr[0]), y(arr[1]), z(arr[2]), w(arr[3]) {};
	vec4(vec3 vec, ld _w = 1) :x(vec.x), y(vec.y), z(vec.z), w(_w) {};

	vec3 toVec3() { return vec3(x, y, z); }
	vec3 toVec3DivW() { return vec3(x / w, y / w, z / w); }
	vec4 operator + (vec4 a) { return vec4(x + a.x, y + a.y, z + a.z, w + a.w); }
	vec4 operator - (vec4 a) { return vec4(x - a.x, y - a.y, z - a.z, w - a.w); }
	vec4 operator / (ld c) { return vec4(x / c, y / c, z / c, w / c); }
	ld operator * (vec4 a) { return (x * a.x + y * a.y + z * a.z, w * a.w); }
	vec4 operator * (ld a) { return vec4(x * a, y * a, z * a, w * a); }
	operator const float* () {
		float* A = new float[4];
		A[0] = x;
		A[1] = y;
		A[2] = z;
		A[3] = w;
		return A;
	}
	inline void operator *= (ld c) {
		x *= c, y *= c, z *= c, w *= c;
	}
	inline void operator /= (ld c) {
		x /= c, y /= c, z /= c, w /= c;
	}
};


class matrix4 {
public:
	//row-major, need to be transposed for OpenGL
	float mt[16];
	matrix4() {
		makeZero();
	}
	matrix4(std::vector<vec4> _mt) {

	}
	float& operator [](int i) {
		return mt[i];
	}
	matrix4 operator + (matrix4 a) {
		matrix4 ret;
		for (int i = 0; i < 16; ++i) ret[i] = mt[i] + a[i];
		return ret;
	}
	matrix4 operator - (matrix4 a) {
		matrix4 ret;
		for (int i = 0; i < 16; ++i) ret[i] = mt[i] - a[i];
		return ret;
	}

	void operator /= (float c) {
		for (int i = 0; i < 16; ++i) mt[i] /= c;
	}
	void operator *= (float c) {
		for (int i = 0; i < 16; ++i) mt[i] *= c;
	}
	void operator += (matrix4 a) {
		for (int i = 0; i < 16; ++i) mt[i] += a[i];
	}

	matrix4 operator * (matrix4 a) const {
		matrix4 ret;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				for (int k = 0; k < 4; ++k) {
					ret[i * 4 + j] += mt[i * 4 + k] * a[k * 4 + j];
				}
		return ret;
	}

	vec4 operator * (vec4 v) {
		vec4 ret;
		ret.x = mt[0] * v.x + mt[1] * v.y + mt[2] * v.z + mt[3] * v.w;
		ret.y = mt[4] * v.x + mt[5] * v.y + mt[6] * v.z + mt[7] * v.w;
		ret.z = mt[8] * v.x + mt[9] * v.y + mt[10] * v.z + mt[11] * v.w;
		ret.w = mt[12] * v.x + mt[13] * v.y + mt[14] * v.z + mt[15] * v.w;
		return ret;
	}

	inline void makeRX(float a) {
		mt[0] = 1.0f;	 mt[1] = 0.0f;        mt[2] = 0.0f;			mt[3] = 0.0f;
		mt[4] = 0.0f;	 mt[5] = std::cos(a); mt[6] = -std::sin(a);	mt[7] = 0.0f;
		mt[8] = 0.0f;	 mt[9] = std::sin(a); mt[10] = std::cos(a);	mt[11] = 0.0f;
		mt[12] = 0.0f;	 mt[13] = 0.0f;        mt[14] = 0.0f;			mt[15] = 1.0f;
	}
	inline void makeRY(float a) {
		mt[0] = std::cos(a); mt[1] = 0.0f; mt[2] = std::sin(a); mt[3] = 0.0f;
		mt[4] = 0.0f;         mt[5] = 1.0f; mt[6] = 0.0f;        mt[7] = 0.0f;
		mt[8] = -std::sin(a); mt[9] = 0.0f; mt[10] = std::cos(a); mt[11] = 0.0f;
		mt[12] = 0.0f;         mt[13] = 0.0f; mt[14] = 0.0f;        mt[15] = 1.0f;
	}
	inline void makeRZ(float a) {
		mt[0] = std::cos(a); mt[1] = -std::sin(a); mt[2] = 0.0f; mt[3] = 0.0f;
		mt[4] = std::sin(a); mt[5] = std::cos(a);  mt[6] = 0.0f; mt[7] = 0.0f;
		mt[8] = 0.0f;        mt[9] = 0.0f;         mt[10] = 1.0f; mt[11] = 0.0f;
		mt[12] = 0.0f;        mt[13] = 0.0f;         mt[14] = 0.0f; mt[15] = 1.0f;
	}
	inline void makeTrans(vec3 t) {
		mt[0] = 1.0f; mt[1] = 0.0f; mt[2] = 0.0f; mt[3] = t.x;
		mt[4] = 0.0f; mt[5] = 1.0f; mt[6] = 0.0f; mt[7] = t.y;
		mt[8] = 0.0f; mt[9] = 0.0f; mt[10] = 1.0f; mt[11] = t.z;
		mt[12] = 0.0f; mt[13] = 0.0f; mt[14] = 0.0f; mt[15] = 1.0f;
	}
	inline void makeScale(vec3 s) {
		mt[0] = s.x;  mt[1] = 0.0f; mt[2] = 0.0f; mt[3] = 0.0f;
		mt[4] = 0.0f; mt[5] = s.y;  mt[6] = 0.0f; mt[7] = 0.0f;
		mt[8] = 0.0f; mt[9] = 0.0f; mt[10] = s.z;  mt[11] = 0.0f;
		mt[12] = 0.0f; mt[13] = 0.0f; mt[14] = 0.0f; mt[15] = 1.0f;
	}
	inline void makeCoord(vec3 x, vec3 y, vec3 z) {
		mt[0] = x.x;  mt[1] = x.y; mt[2] = x.z; mt[3] = 0.0f;
		mt[4] = y.x; mt[5] = y.y;  mt[6] = y.z; mt[7] = 0.0f;
		mt[8] = z.x; mt[9] = z.y; mt[10] = z.z;  mt[11] = 0.0f;
		mt[12] = 0.0f; mt[13] = 0.0f; mt[14] = 0.0f; mt[15] = 1.0f;
	}

	inline void makeOrtho(float l, float r, float b, float t, float n, float f) {
		mt[0]  = 2/(r - l);  mt[1] = 0;			mt[2]  = 0;			mt[3]  = -(r + l) / (r - l);
		mt[4]  = 0;		     mt[5] = 2/(t - b);	mt[6]  = 0;			mt[7]  = -(b + t) / (t - b);
		mt[8]  = 0;		     mt[9] = 0;			mt[10] = 1/(f - n);	mt[11] = - n / (f - n);
		mt[12] = 0.0f;       mt[13] = 0.0f;		mt[14] = 0.0f;	    mt[15] = 1.0f;
	}
	void makeIdentity() {
		makeZero();
		for (int i = 0; i < 4; ++i) mt[i * 4 + i] = 1;
	}
	void makeZero() {
		std::fill(mt, mt + 16, 0);
	}

	inline static matrix4 zero() { matrix4 m; m.makeZero();	     return m; }
	inline static matrix4 Rx(float a) { matrix4 m; m.makeRX(a);	     return m; }
	inline static matrix4 Ry(float a) { matrix4 m; m.makeRY(a);		 return m; }
	inline static matrix4 Rz(float a) { matrix4 m; m.makeRZ(a);		 return m; }
	inline static matrix4 trans(vec3 t) { matrix4 m; m.makeTrans(t);	 return m; }
	inline static matrix4 scale(vec3 s) { matrix4 m; m.makeScale(s);	 return m; }
	inline static matrix4 coord(vec3 x, vec3 y, vec3 z) { matrix4 m; m.makeCoord(x, y, z); return m; }
	inline static matrix4 ortho(float l, float r, float b, float t, float n, float f) { matrix4 m; m.makeOrtho(l,r,b,t,n,f);   return m; }
	inline static matrix4 identity() { matrix4 m; m.makeIdentity();   return m; }

	//Transpose
	inline matrix4 transposed() const {
		matrix4 ret;
		for (int i = 0; i < 4; ++i)
			for (int j = 0; j < 4; ++j)
				ret[j * 4 + i] = mt[i * 4 + j];
		return ret;
	}

	//Get the column major array for OpenGL
	inline float* gl_mat() const {
		return this->transposed().mt;
	}

	//Inverse
	matrix4 inverse() const {
		matrix4 ret;

		ret.mt[0] = mt[5] * mt[10] * mt[15] -
			mt[5] * mt[11] * mt[14] -
			mt[9] * mt[6] * mt[15] +
			mt[9] * mt[7] * mt[14] +
			mt[13] * mt[6] * mt[11] -
			mt[13] * mt[7] * mt[10];

		ret.mt[4] = -mt[4] * mt[10] * mt[15] +
			mt[4] * mt[11] * mt[14] +
			mt[8] * mt[6] * mt[15] -
			mt[8] * mt[7] * mt[14] -
			mt[12] * mt[6] * mt[11] +
			mt[12] * mt[7] * mt[10];

		ret.mt[8] = mt[4] * mt[9] * mt[15] -
			mt[4] * mt[11] * mt[13] -
			mt[8] * mt[5] * mt[15] +
			mt[8] * mt[7] * mt[13] +
			mt[12] * mt[5] * mt[11] -
			mt[12] * mt[7] * mt[9];

		ret.mt[12] = -mt[4] * mt[9] * mt[14] +
			mt[4] * mt[10] * mt[13] +
			mt[8] * mt[5] * mt[14] -
			mt[8] * mt[6] * mt[13] -
			mt[12] * mt[5] * mt[10] +
			mt[12] * mt[6] * mt[9];

		ret.mt[1] = -mt[1] * mt[10] * mt[15] +
			mt[1] * mt[11] * mt[14] +
			mt[9] * mt[2] * mt[15] -
			mt[9] * mt[3] * mt[14] -
			mt[13] * mt[2] * mt[11] +
			mt[13] * mt[3] * mt[10];

		ret.mt[5] = mt[0] * mt[10] * mt[15] -
			mt[0] * mt[11] * mt[14] -
			mt[8] * mt[2] * mt[15] +
			mt[8] * mt[3] * mt[14] +
			mt[12] * mt[2] * mt[11] -
			mt[12] * mt[3] * mt[10];

		ret.mt[9] = -mt[0] * mt[9] * mt[15] +
			mt[0] * mt[11] * mt[13] +
			mt[8] * mt[1] * mt[15] -
			mt[8] * mt[3] * mt[13] -
			mt[12] * mt[1] * mt[11] +
			mt[12] * mt[3] * mt[9];

		ret.mt[13] = mt[0] * mt[9] * mt[14] -
			mt[0] * mt[10] * mt[13] -
			mt[8] * mt[1] * mt[14] +
			mt[8] * mt[2] * mt[13] +
			mt[12] * mt[1] * mt[10] -
			mt[12] * mt[2] * mt[9];

		ret.mt[2] = mt[1] * mt[6] * mt[15] -
			mt[1] * mt[7] * mt[14] -
			mt[5] * mt[2] * mt[15] +
			mt[5] * mt[3] * mt[14] +
			mt[13] * mt[2] * mt[7] -
			mt[13] * mt[3] * mt[6];

		ret.mt[6] = -mt[0] * mt[6] * mt[15] +
			mt[0] * mt[7] * mt[14] +
			mt[4] * mt[2] * mt[15] -
			mt[4] * mt[3] * mt[14] -
			mt[12] * mt[2] * mt[7] +
			mt[12] * mt[3] * mt[6];

		ret.mt[10] = mt[0] * mt[5] * mt[15] -
			mt[0] * mt[7] * mt[13] -
			mt[4] * mt[1] * mt[15] +
			mt[4] * mt[3] * mt[13] +
			mt[12] * mt[1] * mt[7] -
			mt[12] * mt[3] * mt[5];

		ret.mt[14] = -mt[0] * mt[5] * mt[14] +
			mt[0] * mt[6] * mt[13] +
			mt[4] * mt[1] * mt[14] -
			mt[4] * mt[2] * mt[13] -
			mt[12] * mt[1] * mt[6] +
			mt[12] * mt[2] * mt[5];

		ret.mt[3] = -mt[1] * mt[6] * mt[11] +
			mt[1] * mt[7] * mt[10] +
			mt[5] * mt[2] * mt[11] -
			mt[5] * mt[3] * mt[10] -
			mt[9] * mt[2] * mt[7] +
			mt[9] * mt[3] * mt[6];

		ret.mt[7] = mt[0] * mt[6] * mt[11] -
			mt[0] * mt[7] * mt[10] -
			mt[4] * mt[2] * mt[11] +
			mt[4] * mt[3] * mt[10] +
			mt[8] * mt[2] * mt[7] -
			mt[8] * mt[3] * mt[6];

		ret.mt[11] = -mt[0] * mt[5] * mt[11] +
			mt[0] * mt[7] * mt[9] +
			mt[4] * mt[1] * mt[11] -
			mt[4] * mt[3] * mt[9] -
			mt[8] * mt[1] * mt[7] +
			mt[8] * mt[3] * mt[5];

		ret.mt[15] = mt[0] * mt[5] * mt[10] -
			mt[0] * mt[6] * mt[9] -
			mt[4] * mt[1] * mt[10] +
			mt[4] * mt[2] * mt[9] +
			mt[8] * mt[1] * mt[6] -
			mt[8] * mt[2] * mt[5];

		const float det = mt[0] * ret.mt[0] + mt[1] * ret.mt[4] + mt[2] * ret.mt[8] + mt[3] * ret.mt[12];
		ret /= det;
		return ret;
	}

	// 如果要找一個變換 T 的三個軸，先取 T.inverse 再取軸 (T 是 world 打到 local)
	// local 打到 world 就直接取軸

	inline vec3 x_axis() const {
		return vec3(mt[0], mt[4], mt[8]);
	}

	inline vec3 y_axis() const {
		return vec3(mt[1], mt[5], mt[9]);
	}

	inline vec3 z_axis() const {
		return vec3(mt[2], mt[6], mt[10]);
	}

	vec3 operator * (vec3 v) {
		return x_axis() * v.x + y_axis() * v.y + z_axis() * v.z + vec3(mt[3], mt[7], mt[11]);
	}

	inline static matrix4 Rotate(vec3 axis, float ang) {
		matrix4 m;
		auto [x, y, z] = axis;
		ld _c = 1 - cos(ang);
		ld c = cos(ang);
		ld s = sin(ang);
		std::vector<vec3> v = {
			{ x * x * _c + c    , x * y * _c + z * s ,x * z * _c - y * s },
			{ x * y * _c - z * s, y * y * _c + c     ,y * z * _c + x * s },
			{ x * z * _c + y * s, y * z * _c - x * s ,z * z * _c + c }
		};
		for (int i = 0; i < 3; ++i) {
			m[0 + i] = v[i][0];
			m[4 + i] = v[i][1];
			m[8 + i] = v[i][3];
		}
		m[3 * 4 + 3] = 1;
		return m;
	}

	//https://eecs.qmul.ac.uk/~gslabaugh/publications/euler.pdf
	//旋轉分解
	vec3 toEulerAngles() const {
		vec3 ret;
		auto& [psi, theta, phi] = ret;
		if ((mt[3 * 4 + 1] != +1) && (mt[3 * 4 + 1] != -1)) {
			theta = -std::asin(mt[3 * 4 + 1]);
			auto c = std::cos(theta);
			psi = std::atan2(mt[3 * 4 + 2] / c, mt[3 * 4 + 3] / c);
			phi = std::atan2(mt[2 * 4 + 1] / c, mt[1 * 4 + 1] / c);
		}
		else {
			if (mt[3 * 4 + 1] == -1) {
				theta = pi / 2;
				psi = phi + std::atan2(mt[1 * 4 + 2], mt[1 * 4 + 3]);
			}
			else {
				theta = -pi / 2;
				psi = -phi + std::atan2(-mt[1 * 4 + 2], -mt[1 * 4 + 3]);
			}
		}
		return ret;
	}
};

using mat4 = matrix4;


// I/O
inline std::ostream& operator<<(std::ostream& out, const vec3& v) {
	out << v.x << ", " << v.y << ", " << v.z;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const vec4& v) {
	out << v.x << ", " << v.y << ", " << v.z << ", " << v.w;
	return out;
}

inline std::ostream& operator<<(std::ostream& out, const mat4& m) {

	out << m.mt[0] << ", " << m.mt[1] << ", " << m.mt[2] << ", " << m.mt[3] << "\n";
	out << m.mt[4] << ", " << m.mt[5] << ", " << m.mt[6] << ", " << m.mt[7] << "\n";
	out << m.mt[8] << ", " << m.mt[9] << ", " << m.mt[10] << ", " << m.mt[11] << "\n";
	out << m.mt[12] << ", " << m.mt[13] << ", " << m.mt[14] << ", " << m.mt[15] << "\n";
	return out;
}

#ifndef TP(p)
#define TP(p) (p).x,(p).y,(p).z
#endif // !TP(p)


