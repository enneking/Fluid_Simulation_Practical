#include "Camera.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Update()
{
	if (glfwGetKey(window, 'W') == GLFW_PRESS)
	{
		m_vPosition[2] -= m_fCameraSpeed;
	}
	if (glfwGetKey(window, 'S') == GLFW_PRESS)
	{
		m_vPosition[2] += m_fCameraSpeed;
	}
	if (glfwGetKey(window, 'A') == GLFW_PRESS)
	{
		m_vPosition[0] -= m_fCameraSpeed;
	}
	if (glfwGetKey(window, 'D') == GLFW_PRESS)
	{
		m_vPosition[0] += m_fCameraSpeed;
	}
	if (glfwGetKey(window, 'R') == GLFW_PRESS)
	{
		m_vPosition[1] += m_fCameraSpeed;
	}
	if (glfwGetKey(window, 'F') == GLFW_PRESS)
	{
		m_vPosition[1] -= m_fCameraSpeed;
	}
	if (glfwGetKey(window, 'Q') == GLFW_PRESS)
	{
		m_vTarget.
	}
	if (glfwGetKey(window, 'E') == GLFW_PRESS)
	{
		rotateZ(-m_rotationSpeed * 10.0f);
	}

}

void Camera::lookAt()
{
	Eigen::Matrix3f R;
	R.col(2) = (m_vPosition - m_vTarget).normalized();
	R.col(0) = m_vUp.cross(R.col(2)).normalized();
	R.col(1) = R.col(2).cross(R.col(0));
	m_mViewMatrix.topLeftCorner<3, 3>() = R.transpose();
	m_mViewMatrix.topRightCorner<3, 1>() = -R.transpose() * m_vPosition;
	m_mViewMatrix(3, 3) = 1.0f;
}

void Camera::setPerspective(float fovY, float aspect, float near, float far)
{
	float theta = fovY*0.5;
	float range = far - near;
	float invtan = 1. / tan(theta);

	m_mProjectionMatrix(0, 0) = invtan / aspect;
	m_mProjectionMatrix(1, 1) = invtan;
	m_mProjectionMatrix(2, 2) = -(near + far) / range;
	m_mProjectionMatrix(3, 2) = -1;
	m_mProjectionMatrix(2, 3) = -2 * near * far / range;
	m_mProjectionMatrix(3, 3) = 0;
}

void Camera::Rotate(float cx, float cy, float angle, POINT p)
{
	float s = sin(angle);
	float c = cos(angle);

	// translate point back to origin:
	p.x -= cx;
	p.y -= cy;

	// rotate point
	float xnew = p.x * c - p.y * s;
	float ynew = p.x * s + p.y * c;

	// translate point back:
	p.x = xnew + cx;
	p.y = ynew + cy;
	return p;
}