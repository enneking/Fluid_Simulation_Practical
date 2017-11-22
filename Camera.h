#pragma once
#include <GL\glew.h>
#include <GLFW\glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>



#define M_PI 3.1415926535897932384626433832795

class Camera
{
public:
	Camera();
	~Camera();

	void Init();

	void Update(GLFWwindow *window);
	void setPerspective();
	void RotateUp(float angle);
	void RotateRight(float angle);
	void RotateFront(float angle);

	int m_iWidth = 1920;
	int m_iHeight = 1080;

	glm::mat4 m_mProjectionMatrix;
	glm::mat4 m_mViewMatrix;
private:


	glm::vec3 m_vTarget = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_vPosition = glm::vec3(0.0f, 0.0f, 5.0f);
	glm::vec3 m_vFront = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_vRight = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 m_vUp = glm::vec3(0.0f, 1.0f, 0.0f);



	float m_fAspect;
	float m_fNear = 0.1f;
	float m_fFar = 100.f;
	float m_fFoV = 70.0f;



	float m_fCameraSpeed = 0.1f;
};
