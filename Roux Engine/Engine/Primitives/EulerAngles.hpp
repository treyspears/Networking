#ifndef EULERANGLES
#define EULERANGLES

#include <math.h>

template <class T>
class EulerAngles
{
public:
	T yawDegreesAboutZ, pitchDegreesAboutY, rollDegreesAboutX;

	EulerAngles() {}

	EulerAngles(const EulerAngles<T> &rhs) : yawDegreesAboutZ(rhs.yawDegreesAboutZ), pitchDegreesAboutY(rhs.pitchDegreesAboutY), rollDegreesAboutX(rhs.rollDegreesAboutX) {}

	EulerAngles(const T &nYaw, const T &nPitch, const T &nRoll) : yawDegreesAboutZ(nYaw), pitchDegreesAboutY(nPitch), rollDegreesAboutX(nRoll) {}

	EulerAngles &operator =(const EulerAngles<T> &rhs) 
	{
		yawDegreesAboutZ = rhs.yawDegreesAboutZ; pitchDegreesAboutY = rhs.pitchDegreesAboutY; rollDegreesAboutX = rhs.rollDegreesAboutX;
		return *this;
	}

	EulerAngles operator -() 
	{
		return EulerAngles<T>( -yawDegreesAboutZ, -pitchDegreesAboutY, -rollDegreesAboutX );
	}

	EulerAngles &operator +=(const EulerAngles<T> &rhs)
	{
		yawDegreesAboutZ += rhs.yawDegreesAboutZ; pitchDegreesAboutY += rhs.pitchDegreesAboutY; rollDegreesAboutX += rhs.rollDegreesAboutX;
		return *this;
	}


	EulerAngles &operator *=(const T &rhs)
	{
		yawDegreesAboutZ *= rhs.yawDegreesAboutZ; pitchDegreesAboutY *= rhs.pitchDegreesAboutY; rollDegreesAboutX *= rhs.rollDegreesAboutX;
		return *this;
	}

	inline bool		   operator==( const EulerAngles<T>& rhs ) const;
	inline bool		   operator!=( const EulerAngles<T>& rhs ) const;
	inline EulerAngles operator+( const EulerAngles<T>& rhs ) const;
	inline EulerAngles operator-( const EulerAngles<T>& rhs ) const;
	inline EulerAngles operator*( const T& scale ) const;

	static EulerAngles<T> Zero();
};

typedef EulerAngles<float> EulerAnglesf;
typedef EulerAngles<double> EulerAnglesd;

//-----------------------------------------------------------------------------------------------
template <class T>
inline EulerAngles<T> EulerAngles<T>::Zero()
{
	return EulerAngles<T>((T)0, (T)0, (T)0);
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool	EulerAngles<T>::operator==( const EulerAngles<T>& rhs ) const
{
	return ( yawDegreesAboutZ == rhs.yawDegreesAboutZ && pitchDegreesAboutY == rhs.pitchDegreesAboutY && rollDegreesAboutX == rhs.rollDegreesAboutX );
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline bool	EulerAngles<T>::operator!=( const EulerAngles<T>& rhs ) const
{
	return ( yawDegreesAboutZ != rhs.yawDegreesAboutZ || pitchDegreesAboutY != rhs.pitchDegreesAboutY || rollDegreesAboutX != rhs.rollDegreesAboutX );
}


//-----------------------------------------------------------------------------------------------
template <class T>
inline EulerAngles<T> EulerAngles<T>::operator+( const EulerAngles<T>& rhs ) const
{
	return EulerAngles<T>( yawDegreesAboutZ + rhs.yawDegreesAboutZ, pitchDegreesAboutY + rhs.pitchDegreesAboutY, rollDegreesAboutX + rhs.rollDegreesAboutX );
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline EulerAngles<T> EulerAngles<T>::operator-( const EulerAngles<T>& rhs ) const
{
	return EulerAngles<T>( yawDegreesAboutZ - rhs.yawDegreesAboutZ, pitchDegreesAboutY - rhs.pitchDegreesAboutY, rollDegreesAboutX - rhs.rollDegreesAboutX );
}

//-----------------------------------------------------------------------------------------------
template< class T>
inline EulerAngles<T> EulerAngles<T>::operator*( const T& scale ) const
{
	return EulerAngles<T>( yawDegreesAboutZ * scale, pitchDegreesAboutY * scale, rollDegreesAboutX * scale );
}

//-----------------------------------------------------------------------------------------------
template <class T>
inline EulerAngles<T> operator*( const T& scalar, const EulerAngles<T>& eulerAngle )
{
	return EulerAngles<T>( scalar * eulerAngle.yawDegreesAboutZ, scalar * eulerAngle.pitchDegreesAboutY, scalar * eulerAngle.rollDegreesAboutX );
}

#endif