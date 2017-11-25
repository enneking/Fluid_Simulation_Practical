#include "SimSystem.h"

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

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

SPHManager* SimSystem::GetSPHManager()
{
	return &m_oSPHManager;
}

//===================== SYSTEM CORE ======================================


void SimSystem::Run()
{
	m_oSPHManager.Init();

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

            ImGui_ImplGlfwGL3_NewFrame();

            //ImGui::ShowTestWindow();
            
			m_oCamera.Update(m_oWindow);
			m_oSPHManager.Update(m_dt);

            ImGui::Render();
		}
           
		//draw code goes here
		glClearColor(255.0f, 255.0f, 255.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//render our game objects
		m_oSPHManager.GetParticleManager()->DrawParticles();

        // render UI
        auto imguiDrawData = ImGui::GetDrawData();
        ImGui_ImplGlfwGL3_RenderDrawLists(imguiDrawData);

		glfwSwapBuffers(m_oWindow);
		glfwPollEvents();

	}

    ImGui_ImplGlfwGL3_Shutdown();
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

	m_oWindow = glfwCreateWindow(m_oCamera.m_iWidth, m_oCamera.m_iHeight, "Fluid Sim", NULL, NULL);

    if (!ImGui_ImplGlfwGL3_Init(m_oWindow, true)) {
        assert(false);
    }

    ImGui::StyleColorsDark();

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

	m_oCamera.Init();
	m_oSPHManager.GetParticleManager()->Init(&m_oCamera);

}

