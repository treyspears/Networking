#ifndef AABB3_H
#define AABB3_H

#include "Engine/Primitives/Vector3.hpp"

class AABB3
{
public:
	AABB3(const Vector3f& minPoint, const Vector3f& maxPoint);
		
	inline void Translate(const Vector3f& translation);

	inline Vector3f GetCenter();
	inline Vector3f GetMinCoords() const;
	inline Vector3f GetMaxCoords() const;

	inline float GetWidth() const;
	inline float GetHeight() const;
	inline float GetDepth() const;

	//FUTURE EDIT
	Vector3f m_minPoint, m_maxPoint;

private:

	float m_width;
	float m_height;
	float m_depth;
};

//-----------------------------------------------------------------------------------------------
Vector3f AABB3::GetCenter()
{
	return ( ( m_maxPoint + m_minPoint) * 0.5f);
}

//-----------------------------------------------------------------------------------------------
void AABB3::Translate(const Vector3f& translation)
{
	m_minPoint += translation;
	m_maxPoint += translation;
}

//-----------------------------------------------------------------------------------------------
Vector3f AABB3::GetMinCoords() const
{
	return m_minPoint;
}

//-----------------------------------------------------------------------------------------------
Vector3f AABB3::GetMaxCoords() const
{
	return m_maxPoint;
}

//-----------------------------------------------------------------------------------------------
inline float AABB3::GetWidth() const
{
	return m_width;
}

//-----------------------------------------------------------------------------------------------
inline float AABB3::GetHeight() const
{
	return m_height;
}

//-----------------------------------------------------------------------------------------------
inline float AABB3::GetDepth() const
{
	return m_depth;
}

#endif