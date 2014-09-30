#ifndef VECTOR4
#define VECTOR4

#include <math.h>

#include "Engine/Primitives/Matrix44.hpp"

template <class T>
class Vector4
{
public:

	T x, y, z, w;

	Vector4() {}
	Vector4(const Vector4<T> &rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) {}
	Vector4(const T &nX, const T &nY, const T &nZ, const T &nW) : x(nX), y(nY), z(nZ), w(nW) {}

	inline void Zeroize();
	inline void Normalize();

	static Vector4<T> Zero();
	static Vector4<T> GetNormalizedVector(T rhsX, T rhsY, T rhsZ, T rhsW);
	static T Magnitude(const Vector4<T> &vector);
	static T Distance(const Vector4<T> &vectorA, const Vector4<T> &vectorB);
	static Vector4<T> Floor(const Vector4<T> &vector);

	inline Vector4 &operator =(const Vector4<T> &rhs);
	inline bool operator ==(const Vector4<T> &rhs) const;
	inline bool operator !=(const Vector4<T> &rhs) const;	
	inline Vector4 operator -() const;
	inline Vector4 operator +(const Vector4<T> &rhs) const;
	inline Vector4 operator -(const Vector4<T> &rhs) const;
	inline Vector4 operator *(const T &rhs) const;
	inline Vector4 operator /(const T &rhs) const;
	inline Vector4 &operator +=(const Vector4<T> &rhs);
	inline Vector4 &operator -=(const Vector4<T> &rhs);
	inline Vector4 &operator *=(const T &rhs);
	inline Vector4 &operator /=(const T &rhs);
	inline T operator *(const Vector4 &rhs) const;
	inline Vector4 operator *( const Matrix44<T>& rhs ) const;

};

typedef Vector4<float> Vector4f;
typedef Vector4<int> Vector4i;
typedef Vector4<double> Vector4d;


//-----------------------------------------------------------------------------------------------
template <class T>
inline void Vector4<T>::Zeroize()
{ 
	x = y = z = w = (T)0; 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline void Vector4<T>::Normalize()
{
	T magnitudeSquared = x*x + y*y + z*z + w*w;

	if(magnitudeSquared > (T)0)
	{
		T oneOverMagnitude = (T)1 / sqrt(magnitudeSquared);

		x *= oneOverMagnitude;
		y *= oneOverMagnitude;
		z *= oneOverMagnitude;
		w *= oneOverMagnitude;
	}
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::Zero()
{
	return Vector4<T>((T)0, (T)0, (T)0, (T)0);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::GetNormalizedVector(T rhsX, T rhsY, T rhsZ, T rhsW)
{
	Vector4<T> result(rhsX, rhsY, rhsZ, rhsW);
	result.Normalize();

	return result;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector4<T>::Magnitude(const Vector4<T> &vector)
{
	return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z + vector.w * vector.w);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector4<T>::Distance(const Vector4<T> &vectorA, const Vector4<T> &vectorB)
{
	T distanceX = vectorA.x - vectorB.x;
	T distanceY = vectorA.y - vectorB.y;
	T distanceZ = vectorA.z - vectorB.z;
	T distanceW = vectorA.w - vectorB.w;

	return sqrt(distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ + distanceW * distanceW);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::Floor(const Vector4<T> &vector)
{
	return Vector4<T>(floor(vector.x), floor(vector.y), floor(vector.z), floor(vector.w));
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T>& Vector4<T>::operator=(const Vector4<T>& rhs)
{
	x = rhs.x; y = rhs.y; z = rhs.z; w = rhs.w;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector4<T>::operator==(const Vector4<T> &rhs) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector4<T>::operator !=(const Vector4<T> &rhs) const
{
	return x != rhs.x || y != rhs.y || z != rhs.z || w != rhs.w;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::operator -() const 
{
	return Vector4<T>(-x,-y,-z,-w); 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::operator +(const Vector4<T> &rhs) const
{
	return Vector4<T>(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::operator -(const Vector4<T> &rhs) const
{
	return Vector4<T>(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::operator *(const T &rhs) const
{
	return Vector4<T>(x * rhs, y * rhs, z * rhs, w * rhs);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::operator /(const T &rhs) const
{
	T oneOverRhs = (T)1 / rhs;
	return Vector4<T>(x * oneOverRhs, y * oneOverRhs, z * oneOverRhs, w * oneOverRhs);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T>& Vector4<T>::operator +=(const Vector4<T> &rhs)
{
	x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T>& Vector4<T>::operator -=(const Vector4<T> &rhs)
{
	x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T>& Vector4<T>::operator *=(const T &rhs)
{
	x *= rhs; y *= rhs; z *= rhs; w *= rhs.w;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T>& Vector4<T>::operator /=(const T &rhs)
{
	T oneOverRhs = (T)1 / rhs;
	x *= oneOverRhs; y *= oneOverRhs; z *= oneOverRhs; w *= oneOverRhs;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector4<T>::operator *(const Vector4 &rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> operator *(const T &scalar, const Vector4<T> &vector)
{
	return Vector4<T>(scalar * vector.x, scalar * vector.y, scalar * vector.z, scalar * vector.w);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector4<T> Vector4<T>::operator *( const Matrix44<T>& rhs) const
{
	Vector4<T> result;

	result.x = rhs.data[0] * x + rhs.data[4] * y + rhs.data[8] * z + rhs.data[12] * w;
	result.y = rhs.data[1] * x + rhs.data[5] * y + rhs.data[9] * z + rhs.data[13] * w;
	result.z = rhs.data[2] * x + rhs.data[6] * y + rhs.data[10] * z + rhs.data[14] * w;
	result.w = rhs.data[3] * x + rhs.data[7] * y + rhs.data[11] * z + rhs.data[15] * w;

	return result;
}

#endif