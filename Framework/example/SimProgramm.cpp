#include "SimProgramm.h"



SimProgramm::SimProgramm()
{
}


SimProgramm::~SimProgramm()
{
}

void SimProgramm::Run(int argc, char* argv[])
{
	SimSystem::GetInstance()->Init(argc, argv);
	
	m_oNeighborhoodSearch.Run();

	SimSystem::GetInstance()->Run();
}
