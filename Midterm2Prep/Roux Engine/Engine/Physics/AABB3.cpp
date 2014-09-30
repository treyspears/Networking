#include "AABB3.hpp"

//-----------------------------------------------------------------------------------------------
AABB3::AABB3( const Vector3f& minPoint, const Vector3f& maxPoint )
{
	m_minPoint = minPoint;
	m_maxPoint = maxPoint;

	m_width = m_maxPoint.x - m_minPoint.x;
	m_depth = m_maxPoint.y - m_minPoint.y;
	m_height = m_maxPoint.z - m_minPoint.z;
}
