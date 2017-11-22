#pragma once
#include <Eigen\Core>
#include "ParticleManager.h"
#include "CompactNSearch.h"
#include "SPHKernel.h"

class SPHManager
{
public:
	SPHManager();
	~SPHManager();

	void Init();

	void Update(double dt);

	ParticleManager* GetParticleManager();

private:
	void ApplyForces(double dt);
	void ComputeDensityAndPressure();

private:
	double m_fGravityForce = -9.81;
	double m_dStiffness = 0.1;

	double m_dRestDensity = 1000;
	std::vector<double> m_vDensity;
	std::vector<double> m_vPressure;


	ParticleManager m_oParticleManager;
	std::unique_ptr<CompactNSearch> m_oCompactNSearch;
	unsigned int m_iDiscretizationId;
	std::vector<SPHDiscretization> m_vSphDiscretizations;
	
	SPHKernel   m_pSPHKernel;
};


    


