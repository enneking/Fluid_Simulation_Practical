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
        return m_state.density[idx];
    }

    void GUI();


    struct {
        double gravityForce = -9.81;
        double stiffness = 1000.0;
        double simSpeed = 0.01;
        double SPEED_OF_SOUND_POW = 88.5 * 15;
        double restDensity = 1000;
        double particleRadius = .2;
        double smoothingLength = 1.;

        bool   useImprovedBoundaryHandling = true;
    } settings;

private:
	void IntegrationStep(double dt);
    void ApplyViscosity();
	void ComputeDensityAndPressure();

    void ImprovedDensityCalculation();
    void ImprovedBoundaryForceCalculation();
    void PreCalculations();

	void BoundaryForceCalculation();

private:
    struct {
        std::vector<double> weights;
        std::vector<Eigen::Vector3d> deltaWeights;
        std::vector<double> psi;
    } m_precalc;

    struct {
        std::vector<double> density;
        std::vector<double> pressure;
        std::vector<Eigen::Vector3d> boundaryForce;
    } m_state;

	ParticleManager m_oParticleManager;
	std::unique_ptr<CompactNSearch> m_oCompactNSearch;
	unsigned int m_iDiscretizationId;
	std::vector<SPHDiscretization> m_vSphDiscretizations;
	
	SPHKernel   m_pSPHKernel;
};


    


