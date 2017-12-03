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
	void BoundaryForces();
	void ComputeDensityAndPressure();

private:
	double m_fGravityForce = -9.81;
	double m_dStiffness = 0.01;
	double m_iSimSpeed = 0.1;

	double m_dRestDensity = 1000;
	double m_dRadius = 0.1;
	std::vector<double> m_vDensity;
	std::vector<double> m_vPressure;
	std::vector<Eigen::Vector3d> m_vBoundaryForce;
	double m_dSmoothingLenght = 100.0;

	ParticleManager m_oParticleManager;
	std::unique_ptr<CompactNSearch> m_oCompactNSearch;
	unsigned int m_iDiscretizationId;
	std::vector<SPHDiscretization> m_vSphDiscretizations;
	
	SPHKernel   m_pSPHKernel;
};


    


