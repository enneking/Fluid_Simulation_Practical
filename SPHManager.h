#pragma once
#include <Eigen\Core>
#include "ParticleManager.h"
#include "CompactNSearch.h"
#include "SPHKernel.h"

#include <thread>
#include <atomic>
#include <mutex>

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
        return state.density[idx];
    }

    void GUI();


    struct {
        double gravityForce = -9.81;
        double stiffness = 100000.0;
        double restDensity = 1000.0;
        double smoothingLength = 0.2;

        bool   useImprovedBoundaryHandling = true;
    } settings;

    struct WorkGroup
    {
        size_t particleOffset = 0;
        size_t particleCount = 0;

        WorkGroup() = default;
        WorkGroup(size_t offset, size_t count)
            :   particleOffset(offset), particleCount(count) {}
    
    };  

    static const size_t MAX_THREADS = 8;
    struct ThreadContext {
        int id = 0;
        WorkGroup workGroup;
        SPHManager* sph = nullptr;
        //ThreadPool* pool = nullptr;
    } m_threadContext[MAX_THREADS]; // not expecting more than 8 cores 

    double m_dt = 0.0;
    size_t m_numThreads = 0;
    std::atomic_int m_openPressureCounter = 0;
    std::atomic_int m_openBoundaryForceCounter = 0;
    std::atomic_int m_openIntegrationCounter = 0;
    std::atomic_int m_openWorkCounter = 0;
    struct {
        std::vector<double> density;
        std::vector<double> pressure;
        std::vector<double> boundaryD;
        
        std::vector<Eigen::Vector3d> boundaryForce;

        Eigen::Vector3d     externalForceAccum = Eigen::Vector3d(0.0, 0.0, 0.0);
    } state;
    void UpdateWorkGroup(WorkGroup* workGroup, double dt);
	
	SPH::Kernel*    m_pSPHKernel = nullptr;
private:

    // pressure solver
    double EvaluateConstraint(size_t idx);
    Eigen::Vector3d EvaluateConstraintDerivativeJ(size_t i, size_t j);
    Eigen::Vector3d EvaluateConstraintDerivativeI(size_t idx);
    double ComputeLambda(size_t idx);
    Eigen::Vector3d ComputeDisplacement(size_t idx);

	ParticleManager m_oParticleManager;
	std::unique_ptr<CompactNSearch> m_oCompactNSearch;
	unsigned int m_fluidDiscretizationId;
    unsigned int m_boundaryDiscretizationId;
	
	
};


    


