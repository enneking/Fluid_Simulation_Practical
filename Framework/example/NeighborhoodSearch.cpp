#include "NeighborhoodSearch.h"



NeighborhoodSearch::NeighborhoodSearch()
{
}


NeighborhoodSearch::~NeighborhoodSearch()
{
}

void NeighborhoodSearch::Run()
{
	m_vBFNumberNeighbors.reserve(m_iNumberOfParticles);
	m_vBFNeighbouringIndices.reserve(m_iNumberOfParticles);
	m_vBFNumberNeighbors.resize(m_iNumberOfParticles);
	m_vBFNeighbouringIndices.resize(m_iNumberOfParticles);

	SimSystem::GetInstance()->GetParticleManager()->GetParticlePositions()->reserve(m_iNumberOfParticles);

	std::random_device oRandomDevice;
	std::mt19937 Gen(oRandomDevice());
	std::uniform_real_distribution<double> Dis(-30.0, 30.0);
	for (int i = 0; i < m_iNumberOfParticles; i++)
	{
		SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(Dis(Gen), Dis(Gen), Dis(Gen)));
	}
	m_pPosVec = SimSystem::GetInstance()->GetParticleManager()->GetParticlePositions();
	

	begin_CompactNSearch = std::chrono::system_clock::now();
	CompactNSearchApproach();
	end_CompactNSearch = std::chrono::system_clock::now();

	begin_BruteForce = std::chrono::system_clock::now();
	BruteForceApproach();
	end_BruteForce = std::chrono::system_clock::now();

	OutputNeigbhourIndices();

	OutputNumberNeighbours();

	//OutputTimeElapsed();
}

void NeighborhoodSearch::CompactNSearchApproach()
{
	CompactNSearch oCompactNSearch(m_iRadius);
	m_iDiscretizationId = oCompactNSearch.add_discretization(&((*m_pPosVec)[0]), m_pPosVec->size(), false, true);
	oCompactNSearch.neighborhood_search();
	m_vSphDiscretizations = oCompactNSearch.discretizations();

}

void NeighborhoodSearch::BruteForceApproach()
{
	for (int i = 0; i < m_iNumberOfParticles; i++)
	{
		for (int j = i+1; j < m_iNumberOfParticles; j++)
		{
			if (((*m_pPosVec)[i] - (*m_pPosVec)[j]).norm() < m_iRadius)
			{
				m_vBFNumberNeighbors[i] += 1;
				m_vBFNumberNeighbors[j] += 1;
				m_vBFNeighbouringIndices[i].push_back(j);
				m_vBFNeighbouringIndices[j].push_back(i);
			}
		}
	}
}

void NeighborhoodSearch::OutputNumberNeighbours()
{
	std::cout<< "=================== number neighbours ============================" << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < m_iNumberOfParticles; i++)
	{
		std::cout << i  << "_ CompactNSearch: " << m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i) << std::endl;
		std::cout << i <<  "_ BruteForce    : " << m_vBFNumberNeighbors[i] << std::endl;
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void NeighborhoodSearch::OutputNeigbhourIndices()
{
	std::cout<< "================== neighbor indices ===========================:" << std::endl;
	std::cout << std::endl;
	for (int i = 0; i < m_iNumberOfParticles; i++)
	{
		std::cout << i  << "CompactNSearch: ";
		for (int j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
		{
			std::cout << m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index << " | ";
		}
		std::cout << std::endl << std::endl << i <<  "BruteForce    : ";
		for (int j = 0; j < m_vBFNeighbouringIndices[i].size(); j++)
		{
			std::cout << m_vBFNeighbouringIndices[i][j] << " | ";
		}
		std::cout << std::endl << " ----------------------------------------------------------  " << std::endl;
	}
	std::cout << std::endl;
}

void NeighborhoodSearch::OutputTimeElapsed()
{
	std::chrono::duration<double, std::ratio<60>> work_time;

	std::cout << "================== Time Elapsed ===========================:" << std::endl;
	std::cout << std::endl;
	work_time = end_CompactNSearch - begin_CompactNSearch;
	std::cout << "CompactNSearch: " << work_time.count()  << std::endl;
	work_time = end_BruteForce - begin_BruteForce;
	std::cout << "BruteForce    : " << work_time.count() << std::endl << std::endl;
}