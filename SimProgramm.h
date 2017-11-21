#pragma once
#include "SimSystem.h"
#include "SPHKernel.h"
#include "NeighborhoodSearch.h"

class SimProgramm
{
public:
	SimProgramm();
	~SimProgramm();

	void Run(int argc, char* argv[]);

	void InitScene();


private:
	std::unique_ptr<NeighborhoodSearch> m_pNeighborhoodSearch;
	std::unique_ptr<SPHKernel> m_pSPHKernel;
};

