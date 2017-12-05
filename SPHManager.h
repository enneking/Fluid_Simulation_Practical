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

    inline double GetDensityWithIndex(size_t idx)
    {
        return m_vDensity[idx];
    }

    void GUI();

private:
	void ApplyForces(double dt);
    void ApplyViscosity();
	void BoundaryForces();
	void ComputeDensityAndPressure();

    

private:


	double m_fGravityForce = -9.81;
	double m_dStiffness = 1000.0;
	double m_iSimSpeed = 0.01;
	const double SPEED_OF_SOUND_POW = 88.5 * 88.5;
	bool m_bRun = true;

	double m_dRestDensity = 1000;

	double m_dRadius = .2;

	std::vector<double> m_vDensity;
	std::vector<double> m_vPressure;
	std::vector<Eigen::Vector3d> m_vBoundaryForce;
	double m_dSmoothingLength = 1.;

	ParticleManager m_oParticleManager;
	std::unique_ptr<CompactNSearch> m_oCompactNSearch;
	unsigned int m_iDiscretizationId;
	std::vector<SPHDiscretization> m_vSphDiscretizations;
	
	SPHKernel   m_pSPHKernel;
};


    


