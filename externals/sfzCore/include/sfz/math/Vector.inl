// Copyright (c) Peter Hillerstr�m (skipifzero.com, peter@hstroem.se)
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

namespace sfz {

// Vector struct declaration: Vector<T,N>
// ------------------------------------------------------------------------------------------------

template<typename T, size_t N>
Vector<T,N>::Vector(const T* arrayPtr) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		elements[i] = arrayPtr[i];
	}
}

template<typename T, size_t N>
T& Vector<T,N>::operator[] (const size_t index) noexcept
{
	sfz_assert_debug(index < N);
	return elements[index];
}

template<typename T, size_t N>
T Vector<T,N>::operator[] (const size_t index) const noexcept
{
	sfz_assert_debug(index < N);
	return elements[index];
}

// Vector struct declaration: Vector<T,2>
// ------------------------------------------------------------------------------------------------

template<typename T>
Vector<T,2>::Vector(const T* arrayPtr) noexcept
:
	x{arrayPtr[0]},
	y{arrayPtr[1]}
{ }

template<typename T>
Vector<T,2>::Vector(T value) noexcept
:
	x{value},
	y{value}
{ }

template<typename T>
Vector<T,2>::Vector(T x, T y) noexcept
:
	x{x},
	y{y}
{ }

template<typename T>
T& Vector<T,2>::operator[] (const size_t index) noexcept
{
	sfz_assert_debug(index < 2);
	return elements[index];
}

template<typename T>
T Vector<T,2>::operator[] (const size_t index) const noexcept
{
	sfz_assert_debug(index < 2);
	return elements[index];
}

// Vector struct declaration: Vector<T,3>
// ------------------------------------------------------------------------------------------------

template<typename T>
Vector<T,3>::Vector(const T* arrayPtr) noexcept
:
	x{arrayPtr[0]},
	y{arrayPtr[1]},
	z{arrayPtr[2]}
{ }

template<typename T>
Vector<T,3>::Vector(T value) noexcept
:
	x{value},
	y{value},
	z{value}
{ }

template<typename T>
Vector<T,3>::Vector(T x, T y, T z) noexcept
:
	x{x},
	y{y},
	z{z}
{ }

template<typename T>
Vector<T,3>::Vector(Vector<T,2> xy, T z) noexcept
:
	x{xy.elements[0]},
	y{xy.elements[1]},
	z{z}
{ }

template<typename T>
Vector<T,3>::Vector(T x, Vector<T,2> yz) noexcept
:
	x{x},
	y{yz.elements[0]},
	z{yz.elements[1]}
{ }

template<typename T>
T& Vector<T,3>::operator[] (const size_t index) noexcept
{
	sfz_assert_debug(index < 3);
	return elements[index];
}

template<typename T>
T Vector<T,3>::operator[] (const size_t index) const noexcept
{
	sfz_assert_debug(index < 3);
	return elements[index];
}

// Vector struct declaration: Vector<T,4>
// ------------------------------------------------------------------------------------------------

template<typename T>
Vector<T,4>::Vector(const T* arrayPtr) noexcept
:
	x{arrayPtr[0]},
	y{arrayPtr[1]},
	z{arrayPtr[2]},
	w{arrayPtr[3]}
{ }

template<typename T>
Vector<T,4>::Vector(T value) noexcept
:
	x{value},
	y{value},
	z{value},
	w{value}
{ }

template<typename T>
Vector<T,4>::Vector(T x, T y, T z, T w) noexcept
:
	x{x},
	y{y},
	z{z},
	w{w}
{ }

template<typename T>
Vector<T,4>::Vector(Vector<T,3> xyz, T w) noexcept
:
	x{xyz.elements[0]},
	y{xyz.elements[1]},
	z{xyz.elements[2]},
	w{w}
{ }

template<typename T>
Vector<T,4>::Vector(T x, Vector<T,3> yzw) noexcept
:
	x{x},
	y{yzw.elements[0]},
	z{yzw.elements[1]},
	w{yzw.elements[2]}
{ }

template<typename T>
Vector<T,4>::Vector(Vector<T,2> xy, Vector<T,2> zw) noexcept
:
	x{xy.elements[0]},
	y{xy.elements[1]},
	z{zw.elements[0]},
	w{zw.elements[1]}
{ }

template<typename T>
Vector<T,4>::Vector(Vector<T,2> xy, T z, T w) noexcept
:
	x{xy.elements[0]},
	y{xy.elements[1]},
	z{z},
	w{w}
{ }

template<typename T>
Vector<T,4>::Vector(T x, Vector<T,2> yz, T w) noexcept
:
	x{x},
	y{yz.elements[0]},
	z{yz.elements[1]},
	w{w}
{ }

template<typename T>
Vector<T,4>::Vector(T x, T y, Vector<T,2> zw) noexcept
:
	x{x},
	y{y},
	z{zw.elements[0]},
	w{zw.elements[1]}
{ }

template<typename T>
T& Vector<T,4>::operator[] (const size_t index) noexcept
{
	sfz_assert_debug(index < 4);
	return elements[index];
}

template<typename T>
T Vector<T,4>::operator[] (const size_t index) const noexcept
{
	sfz_assert_debug(index < 4);
	return elements[index];
}

// Vector constants
// ------------------------------------------------------------------------------------------------

template<typename T>
Vector<T,3> UNIT_X() noexcept
{
	return Vector<T,3>{T(1), T(0), T(0)};
}

template<typename T>
Vector<T,3> UNIT_Y() noexcept
{
	return Vector<T,3>{T(0), T(1), T(0)};
}

template<typename T>
Vector<T,3> UNIT_Z() noexcept
{
	return Vector<T,3>{T(0), T(0), T(1)};
}

// Vector functions
// ------------------------------------------------------------------------------------------------

template<typename T, size_t N>
T length(const Vector<T,N>& vector) noexcept
{
	return T(std::sqrt(dot(vector, vector)));
}

template<typename T, size_t N>
T squaredLength(const Vector<T,N>& vector) noexcept
{
	return dot(vector, vector);
}

template<typename T, size_t N>
Vector<T,N> normalize(const Vector<T,N>& vector) noexcept
{
	T lengthTmp = length(vector);
	sfz_assert_debug(lengthTmp != 0);
	return vector/lengthTmp;
}

template<typename T, size_t N>
T dot(const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	T product = T(0);
	for (size_t i = 0; i < N; ++i) {
		product += (left.elements[i]*right.elements[i]);
	}
	return product;
}

template<typename T>
T dot(const Vector<T,2>& left, const Vector<T,2>& right) noexcept
{
	return left.x * right.x
	     + left.y * right.y;
}

template<typename T>
T dot(const Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	return left.x * right.x
	     + left.y * right.y
	     + left.z * right.z;
}

template<typename T>
T dot(const Vector<T,4>& left, const Vector<T,4>& right) noexcept
{
	return left.x * right.x
	     + left.y * right.y
	     + left.z * right.z
	     + left.w * right.w;
}

template<typename T>
constexpr Vector<T,3> cross(const Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	return sfz::Vector<T,3>{left.y*right.z - left.z*right.y,
	                        left.z*right.x - left.x*right.z,
	                        left.x*right.y - left.y*right.x};
}

template<typename T, size_t N>
T sum(const Vector<T,N>& vector) noexcept
{
	T result = T(0);
	for (size_t i = 0; i < N; ++i) {
		result += vector.elements[i];
	}
	return result;
}

template<typename T, size_t N>
T angle(const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	T squaredLengthLeft = squaredLength(left);
	sfz_assert_debug(squaredLengthLeft != 0);
	T squaredLengthRight = squaredLength(right);
	sfz_assert_debug(squaredLengthRight != 0);
	return std::acos(dot(left, right)/(std::sqrt(squaredLengthLeft*squaredLengthRight)));
}

template<typename T>
T angle(Vector<T,2> vector) noexcept
{
	sfz_assert_debug(!(vector.x == 0 && vector.y == 0));
	T angle = std::atan2(vector.y, vector.x);
	if (angle < T(0)) {
		angle += T(2)*PI<T>();
	}
	return angle;
}

template<typename T>
Vector<T,2> rotate(Vector<T,2> vector, T angleRadians) noexcept
{
	T cos = std::cos(angleRadians);
	T sin = std::sin(angleRadians);
	return Vector<T,2>{vector.x*cos - vector.y*sin, vector.x*sin + vector.y*cos};
}

template<typename T, size_t N>
Vector<T,N> min(const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	Vector<T,N> temp;
	for (size_t i = 0; i < N; ++i) {
		temp[i] = std::min(left[i], right[i]);
	}
	return temp;
}

template<typename T, size_t N>
Vector<T,N> max(const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	Vector<T,N> temp;
	for (size_t i = 0; i < N; ++i) {
		temp[i] = std::max(left[i], right[i]);
	}
	return temp;
}

template<typename T, size_t N>
Vector<T,N> min(const Vector<T,N>& vector, T scalar) noexcept
{
	Vector<T,N> temp;
	for (size_t i = 0; i < N; ++i) {
		temp[i] = std::min(vector[i], scalar);
	}
	return temp;
}

template<typename T, size_t N>
Vector<T,N> min(T scalar, const Vector<T,N>& vector) noexcept
{
	return min(vector, scalar);
}

template<typename T, size_t N>
Vector<T,N> max(const Vector<T,N>& vector, T scalar) noexcept
{
	Vector<T,N> temp;
	for (size_t i = 0; i < N; ++i) {
		temp[i] = std::max(vector[i], scalar);
	}
	return temp;
}

template<typename T, size_t N>
Vector<T,N> max(T scalar, const Vector<T,N>& vector) noexcept
{
	return max(vector, scalar);
}

template<typename T, size_t N>
Vector<T,N> abs(const Vector<T,N>& vector) noexcept
{
	Vector<T,N> temp;
	for (size_t i = 0; i < N; ++i) {
		temp[i] = std::abs(vector[i]);
	}
	return temp;
}

template<typename T, size_t N>
size_t hash(const Vector<T,N>& vector) noexcept
{
	std::hash<T> hasher;
	size_t hash = 0;
	for (size_t i = 0; i < N; ++i) {
		// hash_combine algorithm from boost
		hash ^= hasher(vector.elements[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}
	return hash;
}

template<size_t N>
StackString toString(const Vector<float,N>& vector, uint32_t numDecimals) noexcept
{
	StackString tmp;
	toString(vector, tmp, numDecimals);
	return tmp;
}

template<size_t N>
void toString(const Vector<float,N>& vector, StackString& string, uint32_t numDecimals) noexcept
{
	static_assert(false, "toString() not implemented for float vectors of this dimension");
}

template<>
inline void toString(const vec2& vector, StackString& string, uint32_t numDecimals) noexcept
{
	StackString32 formatStr;
	formatStr.printf("[%%.%uf, %%.%uf]", numDecimals, numDecimals);
	string.printf(formatStr.str, vector.x, vector.y);
}

template<>
inline void toString(const vec3& vector, StackString& string, uint32_t numDecimals) noexcept
{
	StackString32 formatStr;
	formatStr.printf("[%%.%uf, %%.%uf, %%.%uf]", numDecimals, numDecimals, numDecimals);
	string.printf(formatStr.str, vector.x, vector.y, vector.z);
}

template<>
inline void toString(const vec4& vector, StackString& string, uint32_t numDecimals) noexcept
{
	StackString32 formatStr;
	formatStr.printf("[%%.%uf, %%.%uf, %%.%uf, %%.%uf]", numDecimals, numDecimals, numDecimals, numDecimals);
	string.printf(formatStr.str, vector.x, vector.y, vector.z, vector.w);
}

template<size_t N>
StackString toString(const Vector<int32_t,N>& vector) noexcept
{
	StackString tmp;
	toString(vector, tmp);
	return tmp;
}

template<size_t N>
void toString(const Vector<int32_t,N>& vector, StackString& string) noexcept
{
	static_assert(false, "toString() not implemented for int vectors of this dimension");
}

template<>
inline void toString(const vec2i& vector, StackString& string) noexcept
{
	string.printf("[%i, %i]", vector.x, vector.y);
}

template<>
inline void toString(const vec3i& vector, StackString& string) noexcept
{
	string.printf("[%i, %i, %i]", vector.x, vector.y, vector.z);
}

template<>
inline void toString(const vec4i& vector, StackString& string) noexcept
{
	string.printf("[%i, %i, %i, %i]", vector.x, vector.y, vector.z, vector.w);
}

// Operators (arithmetic & assignment)
// ------------------------------------------------------------------------------------------------

template<typename T, size_t N>
Vector<T,N>& operator+= (Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		left.elements[i] += right.elements[i];
	}
	return left;
}

template<typename T>
Vector<T,2>& operator+= (Vector<T,2>& left, const Vector<T,2>& right) noexcept
{
	left.x += right.x;
	left.y += right.y;
	return left;
}

template<typename T>
Vector<T,3>& operator+= (Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	left.x += right.x;
	left.y += right.y;
	left.z += right.z;
	return left;
}

template<typename T>
Vector<T,4>& operator+= (Vector<T,4>& left, const Vector<T,4>& right) noexcept
{
	left.x += right.x;
	left.y += right.y;
	left.z += right.z;
	left.w += right.w;
	return left;
}

template<typename T, size_t N>
Vector<T,N>& operator-= (Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		left.elements[i] -= right.elements[i];
	}
	return left;
}

template<typename T>
Vector<T,2>& operator-= (Vector<T,2>& left, const Vector<T,2>& right) noexcept
{
	left.x -= right.x;
	left.y -= right.y;
	return left;
}

template<typename T>
Vector<T,3>& operator-= (Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	left.x -= right.x;
	left.y -= right.y;
	left.z -= right.z;
	return left;
}

template<typename T>
Vector<T,4>& operator-= (Vector<T,4>& left, const Vector<T,4>& right) noexcept
{
	left.x -= right.x;
	left.y -= right.y;
	left.z -= right.z;
	left.w -= right.w;
	return left;
}

template<typename T, size_t N>
Vector<T,N>& operator*= (Vector<T,N>& left, T right) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		left.elements[i] *= right;
	}
	return left;
}

template<typename T>
Vector<T,2>& operator*= (Vector<T,2>& left, T right) noexcept
{
	left.x *= right;
	left.y *= right;
	return left;
}

template<typename T>
Vector<T,3>& operator*= (Vector<T,3>& left, T right) noexcept
{
	left.x *= right;
	left.y *= right;
	left.z *= right;
	return left;
}

template<typename T>
Vector<T,4>& operator*= (Vector<T,4>& left, T right) noexcept
{
	left.x *= right;
	left.y *= right;
	left.z *= right;
	left.w *= right;
	return left;
}

template<typename T, size_t N>
Vector<T,N>& operator*= (Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		left.elements[i] *= right.elements[i];
	}
	return left;
}

template<typename T>
Vector<T,2>& operator*= (Vector<T,2>& left, const Vector<T,2>& right) noexcept
{
	left.x *= right.x;
	left.y *= right.y;
	return left;
}

template<typename T>
Vector<T,3>& operator*= (Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	left.x *= right.x;
	left.y *= right.y;
	left.z *= right.z;
	return left;
}

template<typename T>
Vector<T,4>& operator*= (Vector<T,4>& left, const Vector<T,4>& right) noexcept
{
	left.x *= right.x;
	left.y *= right.y;
	left.z *= right.z;
	left.w *= right.w;
	return left;
}

template<typename T, size_t N>
Vector<T,N>& operator/= (Vector<T,N>& left, T right) noexcept
{
	sfz_assert_debug(right != T(0));
	for (size_t i = 0; i < N; ++i) {
		left.elements[i] /= right;
	}
	return left;
}

template<typename T>
Vector<T,2>& operator/= (Vector<T,2>& left, T right) noexcept
{
	sfz_assert_debug(right != T(0));
	left.x /= right;
	left.y /= right;
	return left;
}

template<typename T>
Vector<T,3>& operator/= (Vector<T,3>& left, T right) noexcept
{
	sfz_assert_debug(right != T(0));
	left.x /= right;
	left.y /= right;
	left.z /= right;
	return left;
}

template<typename T>
Vector<T,4>& operator/= (Vector<T,4>& left, T right) noexcept
{
	sfz_assert_debug(right != T(0));
	left.x /= right;
	left.y /= right;
	left.z /= right;
	left.w /= right;
	return left;
}

template<typename T, size_t N>
Vector<T,N>& operator/= (Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		sfz_assert_debug(right.elements[i] != T(0));
		left.elements[i] /= right.elements[i];
	}
	return left;
}

template<typename T>
Vector<T,2>& operator/= (Vector<T,2>& left, const Vector<T,2>& right) noexcept
{
	sfz_assert_debug(right.x != T(0));
	sfz_assert_debug(right.y != T(0));
	left.x /= right.x;
	left.y /= right.y;
	return left;
}

template<typename T>
Vector<T,3>& operator/= (Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	sfz_assert_debug(right.x != T(0));
	sfz_assert_debug(right.y != T(0));
	sfz_assert_debug(right.z != T(0));
	left.x /= right.x;
	left.y /= right.y;
	left.z /= right.z;
	return left;
}

template<typename T>
Vector<T,4>& operator/= (Vector<T,4>& left, const Vector<T,4>& right) noexcept
{
	sfz_assert_debug(right.x != T(0));
	sfz_assert_debug(right.y != T(0));
	sfz_assert_debug(right.z != T(0));
	sfz_assert_debug(right.w != T(0));
	left.x /= right.x;
	left.y /= right.y;
	left.z /= right.z;
	left.w /= right.w;
	return left;
}

// Operators (arithmetic)
// ------------------------------------------------------------------------------------------------

template<typename T, size_t N>
Vector<T,N> operator+ (const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	Vector<T,N> temp = left;
	return (temp += right);
}

template<typename T, size_t N>
Vector<T,N> operator- (const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	Vector<T,N> temp = left;
	return (temp -= right);
}

template<typename T, size_t N>
Vector<T,N> operator- (const Vector<T,N>& vector) noexcept
{
	Vector<T,N> temp = vector;
	return (temp *= T(-1));
}

template<typename T, size_t N>
Vector<T,N> operator* (const Vector<T,N>& left, T right) noexcept
{
	Vector<T,N> temp = left;
	return (temp *= right);
}

template<typename T, size_t N>
Vector<T,N> operator* (const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	Vector<T,N> temp = left;
	return (temp *= right);
}

template<typename T, size_t N>
Vector<T,N> operator* (T left, const Vector<T,N>& right) noexcept
{
	return right * left;
}

template<typename T, size_t N>
Vector<T,N> operator/ (const Vector<T,N>& left, T right) noexcept
{
	Vector<T,N> temp = left;
	return (temp /= right);
}

template<typename T, size_t N>
Vector<T,N> operator/ (const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	Vector<T,N> temp = left;
	return (temp /= right);
}

// Operators (comparison)
// ------------------------------------------------------------------------------------------------

template<typename T, size_t N>
bool operator== (const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	for (size_t i = 0; i < N; ++i) {
		if (left.elements[i] != right.elements[i]) return false;
	}
	return true;
}

template<typename T>
bool operator== (const Vector<T,2>& left, const Vector<T,2>& right) noexcept
{
	return left.x == right.x
	    && left.y == right.y;
}

template<typename T>
bool operator== (const Vector<T,3>& left, const Vector<T,3>& right) noexcept
{
	return left.x == right.x
	    && left.y == right.y
	    && left.z == right.z;
}

template<typename T>
bool operator== (const Vector<T,4>& left, const Vector<T,4>& right) noexcept
{
	return left.x == right.x
	    && left.y == right.y
	    && left.z == right.z
	    && left.w == right.w;
}

template<typename T, size_t N>
bool operator!= (const Vector<T,N>& left, const Vector<T,N>& right) noexcept
{
	return !(left == right);
}

// Standard iterator functions
// ------------------------------------------------------------------------------------------------

template<typename T, size_t N>
T* begin(Vector<T,N>& vector) noexcept
{
	return vector.elements;
}

template<typename T, size_t N>
const T* begin(const Vector<T,N>& vector) noexcept
{
	return vector.elements;
}

template<typename T, size_t N>
const T* cbegin(const Vector<T,N>& vector) noexcept
{
	return vector.elements;
}

template<typename T, size_t N>
T* end(Vector<T,N>& vector) noexcept
{
	return vector.elements + N;
}

template<typename T, size_t N>
const T* end(const Vector<T,N>& vector) noexcept
{
	return vector.elements + N;
}

template<typename T, size_t N>
const T* cend(const Vector<T,N>& vector) noexcept
{
	return vector.elements + N;
}

} // namespace sfz

// Specializations of standard library for sfz::Vector
// ------------------------------------------------------------------------------------------------

namespace std {

template<typename T, size_t N>
size_t hash<sfz::Vector<T,N>>::operator() (const sfz::Vector<T,N>& vector) const noexcept
{
	return sfz::hash(vector);
}

} // namespace std
