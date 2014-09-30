#ifndef VECTOR2
#define VECTOR2

#include <math.h>

template <class T>
class Vector2
{
public:

	T x, y;

	Vector2() {}
	Vector2(const Vector2<T> &rhs) : x(rhs.x), y(rhs.y) {}
	Vector2(const T &nX, const T &nY) : x(nX), y(nY) {}

	inline void Zeroize();
	inline void Normalize();

	static Vector2<T> Zero();
	static Vector2<T> GetNormalizedVector(T rhsX, T rhsY);
	static T Magnitude(const Vector2<T> &vector);
	static T MagnitudeSquared(const Vector2<T> &vector);
	static T Distance(const Vector2<T> &vectorA, const Vector2<T> &vectorB);
	static T DistanceSquared(const Vector2<T> &vectorA, const Vector2<T> &vectorB);
	static Vector2<T> Floor(const Vector2<T> &vector);

	inline Vector2 &operator =(const Vector2<T> &rhs);
	inline bool operator ==(const Vector2<T> &rhs) const;
	inline bool operator !=(const Vector2<T> &rhs) const;	
	inline Vector2 operator -() const;
	inline Vector2 operator +(const Vector2<T> &rhs) const;
	inline Vector2 operator -(const Vector2<T> &rhs) const;
	inline Vector2 operator *(const T &rhs) const;
	inline Vector2 operator /(const T &rhs) const;
	inline Vector2 &operator +=(const Vector2<T> &rhs);
	inline Vector2 &operator -=(const Vector2<T> &rhs);
	inline Vector2 &operator *=(const T &rhs);
	inline Vector2 &operator /=(const T &rhs);
	inline T operator *(const Vector2 &rhs) const;

	//FUTURE EDIT
	inline bool operator <( const Vector2<T> &rhs) const;

};

typedef Vector2<float> Vector2f;
typedef Vector2<int> Vector2i;
typedef Vector2<double> Vector2d;


//-----------------------------------------------------------------------------------------------
template <class T>
inline void Vector2<T>::Zeroize()
{ 
	x = y = z = (T)0; 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline void Vector2<T>::Normalize()
{
	T magnitudeSquared = x*x + y*y;

	if(magnitudeSquared > (T)0)
	{
		T oneOverMagnitude = (T)1 / sqrt(magnitudeSquared);

		x *= oneOverMagnitude;
		y *= oneOverMagnitude;
	}
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::Zero()
{
	return Vector2<T>((T)0, (T)0);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::GetNormalizedVector(T rhsX, T rhsY)
{
	Vector2<T> result(rhsX, rhsY);
	result.Normalize();

	return result;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector2<T>::Magnitude(const Vector2<T> &vector)
{
	return sqrt(vector.x * vector.x + vector.y * vector.y);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector2<T>::MagnitudeSquared(const Vector2<T> &vector)
{
	return vector.x * vector.x + vector.y * vector.y;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector2<T>::Distance(const Vector2<T> &vectorA, const Vector2<T> &vectorB)
{
	T distanceX = vectorA.x - vectorB.x;
	T distanceY = vectorA.y - vectorB.y;

	return sqrt(distanceX * distanceX + distanceY * distanceY);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector2<T>::DistanceSquared(const Vector2<T> &vectorA, const Vector2<T> &vectorB)
{
	T distanceX = vectorA.x - vectorB.x;
	T distanceY = vectorA.y - vectorB.y;

	return distanceX * distanceX + distanceY * distanceY;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::Floor(const Vector2<T> &vector)
{
	return Vector2<T>(floor(vector.x), floor(vector.y));
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T>& Vector2<T>::operator=(const Vector2<T>& rhs)
{
	x = rhs.x; y = rhs.y;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector2<T>::operator==(const Vector2<T> &rhs) const
{
	return x == rhs.x && y == rhs.y;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector2<T>::operator !=(const Vector2<T> &rhs) const
{
	return x != rhs.x || y != rhs.y;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::operator -() const 
{
	return Vector2<T>(-x,-y); 
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::operator +(const Vector2<T> &rhs) const
{
	return Vector2<T>(x + rhs.x, y + rhs.y);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::operator -(const Vector2<T> &rhs) const
{
	return Vector2<T>(x - rhs.x, y - rhs.y);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::operator *(const T &rhs) const
{
	return Vector2<T>(x * rhs, y * rhs);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> Vector2<T>::operator /(const T &rhs) const
{
	T oneOverRhs = (T)1 / rhs;
	return Vector2<T>(x * oneOverRhs, y * oneOverRhs);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T>& Vector2<T>::operator +=(const Vector2<T> &rhs)
{
	x += rhs.x; y += rhs.y;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T>& Vector2<T>::operator -=(const Vector2<T> &rhs)
{
	x -= rhs.x; y -= rhs.y;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T>& Vector2<T>::operator *=(const T &rhs)
{
	x *= rhs; y *= rhs;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T>& Vector2<T>::operator /=(const T &rhs)
{
	T oneOverRhs = (T)1 / rhs;
	x *= oneOverRhs; y *= oneOverRhs;
	return *this;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline T Vector2<T>::operator *(const Vector2 &rhs) const
{
	return x * rhs.x + y * rhs.y;
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline Vector2<T> operator *(const T &scalar, const Vector2<T> &vector)
{
	return Vector2<T>(scalar * vector.x, scalar * vector.y);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool Vector2<T>::operator <(const Vector2<T> &rhs) const
{
	if (y < rhs.y)	return true;
	if(rhs.y < y)	return false;
	if(x < rhs.x)	return true;
	if(rhs.x <= x)	return false;
	return true;
}

#endif