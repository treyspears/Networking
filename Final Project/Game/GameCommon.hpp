#ifndef GAME_COMMON
#define GAME_COMMON


#include "Engine/Primitives/Vector3.hpp"
#include "Engine/Primitives/EulerAngles.hpp"

struct Pose3D
{
	Vector3f	 m_position;
	EulerAnglesf m_orientation;

	Pose3D(){}
	Pose3D( const Vector3f& position, const EulerAnglesf& orientation )
		: m_position( position )
		, m_orientation( orientation )
	{

	}

	Pose3D operator+( const Pose3D& rhs ) const
	{
		Pose3D result;

		result.m_position = m_position + rhs.m_position;
		result.m_orientation = m_orientation + rhs.m_orientation;

		return result;
	}
	Pose3D& operator+=( const Pose3D& rhs )
	{
		m_position += rhs.m_position;
		m_orientation += rhs.m_orientation;

		return *this;
	}

	Pose3D operator-( const Pose3D& rhs ) const
	{
		Pose3D result;

		result.m_position = m_position - rhs.m_position;
		result.m_orientation = m_orientation - rhs.m_orientation;

		return result;
	}

	static const Pose3D Zero();
};

inline const Pose3D Pose3D::Zero()
{
	return Pose3D( Vector3f::Zero(), EulerAnglesf::Zero() );
}


#endif