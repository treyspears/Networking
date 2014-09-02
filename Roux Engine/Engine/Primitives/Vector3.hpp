#ifndef VECTOR3
#define VECTOR3

#include <math.h>

#include "Engine/Primitives/Vector2.hpp"

template <class T>
class Vector3
{
public:

	T x, y, z;

	Vector3() {}
	Vector3(const Vector3<T> &rhs) : x(rhs.x), y(rhs.y), z(rhs.z) {}
	Vector3(const T &nX, const T &nY, const T &nZ) : x(nX), y(nY), z(nZ) {}


	inline void Zeroize();
	inline void Normalize();

	static Vector3<T> Zero();
	static Vector3<T> GetNormalizedVector(T rhsX, T rhsY, T rhsZ);
	static T Magnitude(const Vector3<T> &vector);
	static T MagnitudeSquared(const Vector3<T> &vector);
	static Vector3<T> CrossProduct(const Vector3<T> &vectorA, const Vector3<T> &vectorB);
	static T Distance(const Vector3<T> &vectorA, const Vector3<T> &vectorB);
	static T DistanceSquared(const Vector3<T> &vectorA, const Vector3<T> &vectorB);
	static Vector3<T> Floor(const Vector3<T> &vector);

	inline Vector3 &operator =(const Vector3<T> &rhs);
	inline bool operator ==(const Vector3<T> &rhs) const;
	inline bool operator !=(const Vector3<T> &rhs) const;	
	inline Vector3 operator -() const;
	inline Vector3 operator +(const Vector3<T> &rhs) const;
	inline Vector3 operator +(const Vector2<T> &rhs) const;
	inline Vector3 operator -(const Vector3<T> &rhs) const;
	inline Vector3 operator *(const T &rhs) const;
	inline Vector3 operator /(const T &rhs) const;
	inline Vector3 &operator +=(const Vector3<T> &rhs);
	inline Vector3 &operator -=(const Vector3<T> &rhs);
	inline Vector3 &operator *=(const T &rhs);
	inline Vector3 &operator /=(const T &rhs);
	inline T operator *(const Vector3 &rhs) const;

};

typedef Vector3<float> Vector3f;
typedef Vector3<int> Vector3i;
typedef Vector3<double> Vector3d;


//-----------------------------------------------------------------------------------------------
template <class T>
inline void Vector3<T>::Zeroize()
{ 
	x = y = z = (T)0; 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline void Vector3<T>::Normalize()
{
	T magnitudeSquared = x*x + y*y + z*z;

	if(magnitudeSquared > (T)0)
	{
		T oneOverMagnitude = (T)1 / sqrt(magnitudeSquared);

		x *= oneOverMagnitude;
		y *= oneOverMagnitude;
		z *= oneOverMagnitude;
	}
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::Zero()
{
	return Vector3<T>((T)0, (T)0, (T)0);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::GetNormalizedVector(T rhsX, T rhsY, T rhsZ)
{
	Vector3<T> result(rhsX, rhsY, rhsZ);
	result.Normalize();

	return result;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector3<T>::Magnitude(const Vector3<T> &vector)
{
	return sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector3<T>::MagnitudeSquared(const Vector3<T> &vector)
{
	return ( vector.x * vector.x + vector.y * vector.y + vector.z * vector.z );
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::CrossProduct(const Vector3<T> &vectorA, const Vector3<T> &vectorB)
{
	return Vector3<T>(
		vectorA.y * vectorB.z - vectorA.z * vectorB.y,
		vectorA.z * vectorB.x - vectorA.x * vectorB.z,
		vectorA.x * vectorB.y - vectorA.y * vectorB.x 
		);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector3<T>::Distance(const Vector3<T> &vectorA, const Vector3<T> &vectorB)
{
	T distanceX = vectorA.x - vectorB.x;
	T distanceY = vectorA.y - vectorB.y;
	T distanceZ = vectorA.z - vectorB.z;

	return sqrt(distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector3<T>::DistanceSquared(const Vector3<T> &vectorA, const Vector3<T> &vectorB)
{
	T distanceX = vectorA.x - vectorB.x;
	T distanceY = vectorA.y - vectorB.y;
	T distanceZ = vectorA.z - vectorB.z;

	return distanceX * distanceX + distanceY * distanceY + distanceZ * distanceZ;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::Floor(const Vector3<T> &vector)
{
	return Vector3<T>(floor(vector.x), floor(vector.y), floor(vector.z));
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T>& Vector3<T>::operator=(const Vector3<T>& rhs)
{
	x = rhs.x; y = rhs.y; z = rhs.z;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector3<T>::operator==(const Vector3<T> &rhs) const
{
	return x == rhs.x && y == rhs.y && z == rhs.z;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector3<T>::operator !=(const Vector3<T> &rhs) const
{
	return x != rhs.x || y != rhs.y || z != rhs.z;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::operator -() const 
{
	return Vector3<T>(-x,-y,-z); 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::operator +(const Vector3<T> &rhs) const
{
	return Vector3<T>(x + rhs.x, y + rhs.y, z + rhs.z);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::operator +(const Vector2<T> &rhs) const
{
	return Vector3<T>(x + rhs.x, y + rhs.y, z);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::operator -(const Vector3<T> &rhs) const
{
	return Vector3<T>(x - rhs.x, y - rhs.y, z - rhs.z);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::operator *(const T &rhs) const
{
	return Vector3<T>(x * rhs, y * rhs, z * rhs);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> Vector3<T>::operator /(const T &rhs) const
{
	T oneOverRhs = (T)1 / rhs;
	return Vector3<T>(x * oneOverRhs, y * oneOverRhs, z * oneOverRhs);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T>& Vector3<T>::operator +=(const Vector3<T> &rhs)
{
	x += rhs.x; y += rhs.y; z += rhs.z;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T>& Vector3<T>::operator -=(const Vector3<T> &rhs)
{
	x -= rhs.x; y -= rhs.y; z -= rhs.z;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T>& Vector3<T>::operator *=(const T &rhs)
{
	x *= rhs; y *= rhs; z *= rhs;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T>& Vector3<T>::operator /=(const T &rhs)
{
	T oneOverRhs = (T)1 / rhs;
	x *= oneOverRhs; y *= oneOverRhs; z *= oneOverRhs;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector3<T>::operator *(const Vector3 &rhs) const
{
	return x * rhs.x + y * rhs.y + z * rhs.z;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector3<T> operator *(const T &scalar, const Vector3<T> &vector)
{
	return Vector3<T>(scalar * vector.x, scalar * vector.y, scalar * vector.z);
}

#endif