#include "Camera.h"
#include <Eigen\Core>
#include <Eigen\Geometry>
#include <iostream>

#include "imgui.h"

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::Init()
{
	m_fAspect = (float)m_iWidth / (float)m_iHeight;
	setPerspective();
	m_mViewMatrix = glm::lookAt(m_vPosition, m_vTarget, m_vUp);
}

void Camera::Update(GLFWwindow *window)
{
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            m_vPosition += m_fCameraSpeed * m_vFront;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            m_vPosition -= m_fCameraSpeed * m_vFront;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            m_vPosition -= m_fCameraSpeed * m_vRight;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            m_vPosition += m_fCameraSpeed * m_vRight;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            m_vPosition += m_fCameraSpeed * m_vUp;
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            m_vPosition -= m_fCameraSpeed * m_vUp;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            RotateUp(0.7f);
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            RotateUp(-0.7f);
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            RotateRight(0.7f);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            RotateRight(-0.7f);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            RotateFront(-0.7f);
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            RotateFront(0.7f);
        }
    }
	m_vTarget = m_vFront + m_vPosition;

	m_mViewMatrix = glm::lookAt(m_vPosition, m_vTarget, m_vUp);

	//std::cout << "\n view : " << m_mViewMatrix[3][0] << " " << m_mViewMatrix[3][1] << " " << m_mViewMatrix[3][2];
	//std::cout << "\n pos : " << m_vPosition[0] << " " << m_vPosition[1] << " " << m_vPosition[2];
}


void Camera::setPerspective()
{
	float phiHalfInRadians = 0.5f * m_fFoV * ((float)M_PI / 180.0f);
	float t = m_fNear * tan(phiHalfInRadians);
	float b = -t;
	float left = b * m_fAspect;
	float right = t * m_fAspect;

	glViewport(0, 0, m_iWidth, m_iHeight);
	m_mProjectionMatrix = glm::frustum(left, right, b, t, m_fNear, m_fFar);
}

void Camera::RotateRight(float angle)
{
	//Rotate viewdir around the right vector:
	m_vFront = glm::normalize(m_vFront * cos(angle * M_PI / 180.0f)
		+ m_vUp * sin(angle * M_PI / 180.0f));

	//now compute the new UpVector (by cross product)
	m_vUp = glm::cross(m_vFront, m_vRight) * (-1.0f);
}

void Camera::RotateUp(float angle)
{
	m_vFront = glm::normalize(m_vFront * cos(angle * M_PI / 180.0f)
		- m_vRight * sin(angle * M_PI / 180.0f));

	m_vRight = glm::cross(m_vFront, m_vUp);
}

void Camera::RotateFront(float angle)
{
	m_vRight = glm::normalize(m_vRight * cos(angle * M_PI / 180.0f)
		+ m_vUp * sin(angle * M_PI / 180.0f));

	m_vUp = glm::cross(m_vFront, m_vRight) * (-1.0f);
}