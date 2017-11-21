#pragma once
#include "SimSystem.h"


#define M_PI 3.1415926535897932384626433832795

class Camera
{
public:
	Camera();
	~Camera();

	void Update();
	void setPerspective(float fovY, float aspect, float near, float far);
	void lookAt();
	void Rotate();

private:
	Eigen::Matrix3f m_mProjectionMatrix;
	Eigen::Matrix3f m_mViewMatrix;

	Eigen::Vector3f m_vTarget;
	Eigen::Vector3f m_vPosition;
	Eigen::Vector3f m_vUp;

	float m_fCameraSpeed;
};
