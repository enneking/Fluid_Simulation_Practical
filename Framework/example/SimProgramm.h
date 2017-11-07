#pragma once
#include "SimSystem.h"
#include "NeighborhoodSearch.h"

class SimProgramm
{
public:
	SimProgramm();
	~SimProgramm();

	void Run(int argc, char* argv[]);


private:
	NeighborhoodSearch m_oNeighborhoodSearch;
};

