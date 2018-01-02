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
        return m_state.density[idx];
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

private:
    void ComputeNeighborhoods();
	void IntegrationStep(WorkGroup workGroup, double dt);
	void ComputeDensityAndPressure(WorkGroup workGroup);

    void ImprovedDensityCalculation(WorkGroup workGroup);
    void ImprovedBoundaryForceCalculation(WorkGroup workGroup);
    //void PreCalculations(WorkGroup workGroup);

	void BoundaryForceCalculation(WorkGroup workGroup);

    void ApplyViscosity(WorkGroup workGroup);

private:
    /*struct {
		std::vector<std::vector<double>> weights;
		std::vector<std::vector<Eigen::Vector3d>> deltaWeights;
        std::vector<double> psi;
    } m_precalc;*/

    struct {
        std::vector<double> density;
        std::vector<double> pressure;
        std::vector<Eigen::Vector3d> boundaryForce;
    } m_state;

	ParticleManager m_oParticleManager;
	std::unique_ptr<CompactNSearch> m_oCompactNSearch;
	unsigned int m_fluidDiscretizationId;
    unsigned int m_boundaryDiscretizationId;
	

    typedef void(*TaskFunc)(void*);
    struct Task
    {
        TaskFunc func = nullptr;
        void* data = nullptr;
        Task() = default;
        Task(TaskFunc f, void* d)
            :   func(f), data(d) {}
    };
    struct {
        std::vector<std::unique_ptr<std::thread>> threads;
        std::mutex workMutex;
        std::vector<Task>  workQueue;
        std::atomic<uint32_t> workQueueSize = 0;
        std::atomic<bool> exit = false;

       
        void PushTask(Task work)
        {
            {
                std::lock_guard<std::mutex> lock(workMutex);
                workQueue.push_back(work);
            }
            workQueueSize++;
        }

        bool FetchTask(Task* outWork)
        {
            {
                std::lock_guard<std::mutex> lock(workMutex);
                if (workQueue.empty()) { return false; }
                *outWork = workQueue.back();
                workQueue.pop_back();
            }
            return true;
        }
    } m_threadpool;

	SPH::Kernel*    m_pSPHKernel = nullptr;
};


    


