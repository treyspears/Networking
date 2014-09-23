#ifndef BODY
#define BODY

//-----------------------------------------------------------------------------------------------
class Body
{
public:

	Body();
	Body( float initialRadius );

	inline float GetRadius() const;
	inline void SetRadius( float newRadius );

private:

	float m_radius;
};

//-----------------------------------------------------------------------------------------------
inline Body::Body() 
	: m_radius( 0.f )
{

}

//-----------------------------------------------------------------------------------------------
inline Body::Body(  float initialRadius )
	: m_radius( initialRadius )
{

}

//-----------------------------------------------------------------------------------------------
float Body::GetRadius() const
{
	return m_radius;
}

//-----------------------------------------------------------------------------------------------
void Body::SetRadius( float newRadius )
{
	m_radius = newRadius;
}


#endif