#ifndef CAMERA_COORDINATOR
#define CAMERA_COORDINATOR

#include "Camera3D.hpp"


//-----------------------------------------------------------------------------------------------
class CameraCoordinator
{
public:

	//const Camera3D& GetBlendedCamera() const;

private:
	
	std::vector< Camera3D* > m_cameras;
};

#endif