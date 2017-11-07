#include "SimProgramm.h"

#include "SPHKernel.h"

SimProgramm::SimProgramm()
{
}


SimProgramm::~SimProgramm()
{
}

void SimProgramm::Run(int argc, char* argv[])
{

    SPHKernel kernel;

	SimSystem::GetInstance()->Init(argc, argv);
	
	m_oNeighborhoodSearch.Run();

	SimSystem::GetInstance()->Run();
}
