#include "SimProgramm.h"


SimProgramm::SimProgramm()
{
}


SimProgramm::~SimProgramm()
{
}

void SimProgramm::Run(int argc, char* argv[])
{
	//m_pNeighborhoodSearch = std::make_unique<NeighborhoodSearch>();
	m_pSPHKernel = std::make_unique<SPHKernel>();

	SimSystem::GetInstance()->Init(argc, argv);
	
	//m_pNeighborhoodSearch->Run();
	m_pSPHKernel->Run();

	SimSystem::GetInstance()->Run();
}
