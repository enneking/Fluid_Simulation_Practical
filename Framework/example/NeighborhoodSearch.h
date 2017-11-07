#pragma once
#include "SimSystem.h"
#include <random>
#include "CompactNSearch.h"
#include <iostream>
#include <chrono>

class NeighborhoodSearch
{
public:
	NeighborhoodSearch();
	~NeighborhoodSearch();

	void Run();

	void CompactNSearchApproach();
	void BruteForceApproach();

	void OutputNumberNeighbours();
	void OutputNeigbhourIndices();
	void OutputTimeElapsed();

private:
	const int m_iNumberOfParticles = 100;

	std::vector<SPHDiscretization> m_vSphDiscretizations;
	std::vector<Eigen::Vector3d>* m_pPosVec;
	unsigned int m_iDiscretizationId;
	std::vector<int> m_vBFNumberNeighbors;
	std::vector<std::vector<int>> m_vBFNeighbouringIndices;

	std::chrono::system_clock::time_point begin_CompactNSearch;
	std::chrono::system_clock::time_point end_CompactNSearch;
	std::chrono::system_clock::time_point begin_BruteForce;
	std::chrono::system_clock::time_point end_BruteForce;
 
	

	const float m_iRadius = 10.0f;
};

