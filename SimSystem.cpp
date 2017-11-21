#include "SimSystem.h"

SimSystem* SimSystem::s_oSimSystem = NULL;

SimSystem::SimSystem()
{
}

SimSystem::~SimSystem()
{
	
}

SimSystem* SimSystem::GetInstance()
{
	if (s_oSimSystem == NULL)
	{
		s_oSimSystem = new SimSystem();
	}
	return s_oSimSystem;
}


//=================== GETTER FUNCTIONS ==============================

ParticleManager* SimSystem::GetParticleManager()
{
	return &m_oParticleManager;
}

//===================== SYSTEM CORE ======================================


void SimSystem::Run()
{
	m_oParticleManager.InitBuffers();

	std::chrono::system_clock::time_point CurrentTime = std::chrono::system_clock::now();
	std::chrono::system_clock::time_point NewTime;
	std::chrono::duration<double> LagTime;

	double accumulator = m_dt;

	while (glfwGetKey(m_oWindow, GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{

		NewTime = std::chrono::system_clock::now();
		LagTime = std::chrono::duration_cast<std::chrono::duration<double>>(NewTime - CurrentTime);
		if (LagTime.count() > 0.25)
			accumulator += 0.25;
		CurrentTime = NewTime;
		accumulator += LagTime.count();


		while (accumulator >= m_dt)
		{
			accumulator -= m_dt;

			//m_ParticleManager.updateParticles(dt);
		}

		//draw code goes here
		glClearColor(255.0f, 255.0f, 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render our game objects
		m_oParticleManager.DrawParticles();

		glfwSwapBuffers(m_oWindow);
		glfwPollEvents();

	}
	glfwDestroyWindow(m_oWindow);
	glfwTerminate();
}


void SimSystem::Init(int argc, char* argv[])
{

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_SAMPLES, 4); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	m_oWindow = glfwCreateWindow(m_iWidth, m_iHeight, "Flip", NULL, NULL);

	//errorcheck
	if (!m_oWindow)
	{
		fprintf(stderr, "Failed to create GLFW window! \n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//setup context
	glfwMakeContextCurrent(m_oWindow);
	glfwSwapInterval(1);
	//--------------------

	/// ---- initialize glew ----------
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(EXIT_FAILURE);
	}

	m_oParticleManager.Init();
}

