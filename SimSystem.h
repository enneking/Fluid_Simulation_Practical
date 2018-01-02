#pragma once
#include <memory>
#include <chrono>
#include "Camera.h"
#include "SPHManager.h"


class SimSystem
{
public:
	static SimSystem* GetInstance();

	void Init(int argc, char* argv[]);
	void Run();

	SPHManager* GetSPHManager();



private:	
	SimSystem::SimSystem();
	SimSystem::~SimSystem();

	SPHManager m_oSPHManager;

	GLFWwindow* m_oWindow;
	Camera m_oCamera;

	static SimSystem* s_oSimSystem;

	const double m_dt = 1.0 / 800.0;
	const double RenderFPS = 1.0 / 60.0;
	const double StepsPerFrame = RenderFPS / m_dt;
};

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}