#include "SimSystem.h"

SimSystem SimSystem::s_oSimSystem;

SimSystem::SimSystem()
{
}


SimSystem::~SimSystem()
{
	
}

SimSystem* SimSystem::GetInstance()
{
	return &s_oSimSystem;
}


//=================== GETTER FUNCTIONS ==============================

ParticleManager* SimSystem::GetParticleManager()
{
	return &m_oParticleManager;
}

SimViz* SimSystem::GetSimViz()
{
	return m_pSimViz.get();
}



//===================== SYSTEM CORE ======================================

void TW_CALL reset_simulation(void*)
{
	SimSystem::GetInstance()->GetSimViz()->g_time = 0.0f;
}

void TW_CALL export_frame(void*)
{
	GLviz::export_frame("export");
}

void SimSystem::Run()
{
	GLviz::exec(m_pSimViz->camera);
}

void SimSystem::Init(int argc, char* argv[])
{
	GLviz::init(argc, argv);

	this->m_pSimViz = std::make_unique<SimViz>();

	{
		TwBar* bar = GLviz::twbar();

		TwAddVarRO(bar, "time", TW_TYPE_FLOAT,
			&m_pSimViz->g_time, " precision=3 label='t in sec' group='Simulation' ");

		TwAddButton(bar, "Reset",
			reset_simulation, NULL,
			" key=r help='Reset simulation' group='Simulation' ");

		TwAddVarRW(bar, "Stop", TW_TYPE_BOOLCPP,
			&m_pSimViz->g_stop_simulation,
			" key=SPACE help='Stop simulation' group='Simulation' ");

		TwAddVarRW(bar, "Draw Triangle Mesh", TW_TYPE_BOOLCPP,
			&m_pSimViz->g_enable_mesh3,
			" key=1 help='Draw Triangle Mesh' group='Triangle Mesh' ");

		TwType shading_type = TwDefineEnumFromString("shading_type", "Flat,Phong");
		TwAddVarRW(bar, "Shading", shading_type, &m_pSimViz->g_shading_method, " key=5 group='Triangle Mesh' ");

		TwAddSeparator(bar, NULL, " group='Triangle Mesh' ");

		TwAddVarRW(bar, "Wireframe", TW_TYPE_BOOLCPP,
			&m_pSimViz->g_enable_wireframe,
			" key=w help='Draw Wireframe' group='Triangle Mesh' ");

		TwAddVarRW(bar, "Wireframe Color", TW_TYPE_COLOR3F,
			m_pSimViz->g_wireframe,
			" help='Wireframe Color' group='Triangle Mesh' ");

		TwAddSeparator(bar, NULL, " group='Triangle Mesh' ");

		TwAddVarRW(bar, "Mesh Material", TW_TYPE_COLOR3F,
			m_pSimViz->g_mesh_material,
			" help='Mesh Ambient' group='Triangle Mesh' ");

		TwAddVarRW(bar, "Mesh Shininess", TW_TYPE_FLOAT,
			&m_pSimViz->g_mesh_material[3],
			" min=1e-12 max=1000 help='Mesh Shininess' group='Triangle Mesh' ");

		TwAddVarRW(bar, "Draw Points", TW_TYPE_BOOLCPP,
			&m_pSimViz->g_enable_points,
			" key=2 help='Draw Points' group='Points' ");

		TwAddVarRW(bar, "Radius", TW_TYPE_FLOAT,
			&m_pSimViz->g_point_radius,
			" min=0 max=0.1 step=0.0001 key=2 help='Radius' group='Points' ");

		TwAddVarRW(bar, "Points Material", TW_TYPE_COLOR3F,
			m_pSimViz->g_points_material,
			" help='Points Ambient' group='Points' ");

		TwAddVarRW(bar, "Points Shininess", TW_TYPE_FLOAT,
			&m_pSimViz->g_points_material[3],
			" min=1e-12 max=1000 help='Points Shininess' group='Points' ");

		TwAddButton(bar, "Export Frame", export_frame, nullptr, nullptr);
	}

	GLviz::close_callback(std::bind(&SimSystem::closeFunc, this));
}

void SimSystem::closeFunc()
{
	m_pSimViz.get_deleter();
}
