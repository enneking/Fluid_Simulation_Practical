#pragma once
#include "ParticleManager.h"
#include <memory>
#include <chrono>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include "Camera.h"


class SimSystem
{
public:
	static SimSystem* GetInstance();

	void Init(int argc, char* argv[]);
	void Run();

	ParticleManager* GetParticleManager();


private:	
	SimSystem::SimSystem();
	SimSystem::~SimSystem();

	GLFWwindow* m_oWindow;
	ParticleManager m_oParticleManager;
	static SimSystem* s_oSimSystem;

	const double m_dt = 1.0 / 60.0;
	int m_iWidth = 1920;
	int m_iHeight = 1080;
};

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}