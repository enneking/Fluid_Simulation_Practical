#include "SPHManager.h"
#include "imgui.h"


SPHManager::SPHManager()
{
}


SPHManager::~SPHManager()
{
    m_threadpool.exit = true;
    for (auto& thread : m_threadpool.threads) {
        thread->join();
    }
}

ParticleManager* SPHManager::GetParticleManager()
{
	return &m_oParticleManager;
}

void SPHManager::Init()
{
    /*
    for (size_t i = 0; i < 1024; ++i) {
        auto posX = (i / 1024.0) * 4.0 - 2;
        m_oParticleManager.AddParticle(Eigen::Vector3d(posX, 0.0, 0.0));
    }
    */

	m_oParticleManager.InitBuffers();
	m_state.density.resize(m_oParticleManager.GetParticleContainer()->size());
	m_state.pressure.resize(m_oParticleManager.GetParticleContainer()->size());
	m_state.boundaryForce.resize(m_oParticleManager.GetParticleContainer()->size());

	m_oCompactNSearch = std::make_unique<CompactNSearch>(settings.particleRadius);
	m_iDiscretizationId = m_oCompactNSearch->add_discretization(
		&(*m_oParticleManager.GetParticlePositions())[0],
		m_oParticleManager.GetParticlePositions()->size(),
		false, true);

    m_precalc.weights.resize(m_oParticleManager.GetParticleContainer()->size() * m_oParticleManager.GetParticlePositions()->size());
    m_precalc.deltaWeights.resize(m_precalc.weights.size());
    m_precalc.psi.resize(m_oParticleManager.GetBoundarieParticleCount());


    /*for (auto i = 0; i < std::thread::hardware_concurrency() - 1; ++i) {
        m_threadpool.threads.emplace_back(std::make_unique<std::thread>([this] {
            do {
                if (m_threadpool.workQueueSize > 0) {
                    Task task;
                    if (m_threadpool.FetchTask(&task)) {
                        task.func(task.data);
                        m_threadpool.workQueueSize--;
                    }
                }
            } while (!m_threadpool.exit);
        }));
    }*/
    
}



void ApplyViscosity(ParticleManager::Particle* particles, Eigen::Vector3d* positions, double* densities, size_t numParticles, double particleMass, SPHKernel* kernel, SPHDiscretization* discretization, double smoothingLength)
{
    constexpr double epsilon = 0.05;
    for (auto pidx = 0; pidx < numParticles; ++pidx) {
        auto& p = particles[pidx];
        auto vPrime = p.m_vVelocity;
        for (unsigned int didx = 0; didx < discretization->n_neighbors(pidx); didx++)
        {
            int j = discretization->neighbor(pidx, didx).index;
            if (j < numParticles)
            {

                vPrime += (particleMass / densities[j]) * (particles[j].m_vVelocity - p.m_vVelocity) * kernel->QuadricSmoothingFunctionKernel(positions[pidx] - positions[j], smoothingLength);
            }
        }
        vPrime *= epsilon;
        p.m_vVelocity = vPrime;
    }
}



void SPHManager::Update(double dt)
{

	dt *= settings.simSpeed;

    PreCalculations();

	ComputeDensityAndPressure();

    //BoundaryForces();
    if (settings.useImprovedBoundaryHandling) {
        ImprovedBoundaryForceCalculation();
    }
    else {
        BoundaryForceCalculation();
    }
    IntegrationStep(dt);
    ApplyViscosity(m_oParticleManager.GetParticleContainer()->data(), m_oParticleManager.GetParticlePositions()->data(), m_state.density.data(), m_oParticleManager.GetParticleContainer()->size(), m_oParticleManager.GetParticleMass(), &m_pSPHKernel, &m_vSphDiscretizations[m_iDiscretizationId], settings.smoothingLength);
}




void SPHManager::GUI()
{

    if (ImGui::Begin("SPHManager")) {
        struct DensityPlotData
        {
            SPHManager* self = nullptr;
        } densityPlotData = { this };
        ImGui::PlotHistogram("Density Samples", [](void* data, int idx) -> float {
            auto densityPlotData = static_cast<DensityPlotData*>(data);
            auto density = densityPlotData->self->GetDensityWithIndex(idx);
            return (float)density;
        }, &densityPlotData, m_oParticleManager.GetParticleContainer()->size(), /*m_oParticleManager.GetParticleContainer()->size() - 1024*/0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 200));


        ImGui::PlotHistogram("Boundary Forces", [](void* data, int idx) -> float {
            Eigen::Vector3d* forces = static_cast<Eigen::Vector3d*>(data);
            return (float)forces[idx].norm();
        }, m_state.boundaryForce.data(), m_state.boundaryForce.size());

        if (ImGui::TreeNode("Settings")) {

            float gravity = (float)settings.gravityForce;
            float stiffness = (float)settings.stiffness;
            float simSpeed = (float)settings.simSpeed;
            float restDens = (float)settings.restDensity;
            float mass = (float)m_oParticleManager.GetParticleMass();
            float radius = (float)settings.particleRadius;
            float smoothingLength = (float)settings.smoothingLength;

            ImGui::DragFloat("Gravity", &gravity, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat("Stiffness", &stiffness, 1.0f);
            ImGui::DragFloat("Simulation Speed", &simSpeed, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat("Rest Density", &restDens, 0.01f);
            ImGui::DragFloat("Mass", &mass, 0.01f);
            ImGui::DragFloat("Kernel Radius", &radius, 0.01);
            ImGui::DragFloat("Smoothing Length", &smoothingLength, 0.01f, 0.1f, 1.0f);
            ImGui::Checkbox("Use improved boundary handling", &settings.useImprovedBoundaryHandling);

            settings.gravityForce = (double)gravity;
            settings.stiffness = (double)stiffness;
            settings.simSpeed = (double)simSpeed;
            settings.restDensity = (double)restDens;
            m_oParticleManager.SetParticleMass((double)mass);
            settings.smoothingLength = smoothingLength;

            settings.particleRadius = (double)radius;
            m_oCompactNSearch->set_radius(settings.particleRadius);

            ImGui::TreePop();

        }

    } ImGui::End();
}


void SPHManager::IntegrationStep(double dt)
{
    
	for (unsigned int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	{
		Eigen::Vector3d acceleration(0, 0, 0);
		for (unsigned int j = 0; (j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i)); j++)
		{
			int index = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;
			if (index < m_oParticleManager.GetParticleContainer()->size())
			{	
				acceleration += m_oParticleManager.GetParticleMass() * ((m_state.pressure[i] / (m_state.density[i] * m_state.density[i])) + (m_state.pressure[index] / (m_state.density[index] * m_state.density[index])))
					* m_pSPHKernel.QuadricSmoothingFunctionKernelGradient((*m_oParticleManager.GetParticlePositions())[i] - (*m_oParticleManager.GetParticlePositions())[index], settings.smoothingLength);
			}
		}
        acceleration *= -1.0;
		acceleration += (1.0 / m_oParticleManager.GetParticleMass()) * m_state.boundaryForce[i];
		acceleration[1] += settings.gravityForce;
		auto vel = m_oParticleManager.GetParticleContainer()->data()[i].m_vVelocity += dt * (acceleration);
        m_oParticleManager.GetParticlePositions()->at(i) += vel * dt;
    }
}

void SPHManager::ComputeDensityAndPressure()
{
	m_oCompactNSearch->neighborhood_search();
	m_vSphDiscretizations = m_oCompactNSearch->discretizations();

    if (settings.useImprovedBoundaryHandling) {
        ImprovedDensityCalculation();
    }
    for (int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
    {
        if (!settings.useImprovedBoundaryHandling) {
            for (unsigned int j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
            {
                if (m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index < m_oParticleManager.GetParticleContainer()->size())
                {
                    m_state.density[i] += m_oParticleManager.GetParticleMass() * m_pSPHKernel.QuadricSmoothingFunctionKernel((*m_oParticleManager.GetParticlePositions())[i]
                        - (*m_oParticleManager.GetParticlePositions())[m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index], settings.smoothingLength);
                }

            }
        }
        m_state.pressure[i] = glm::max(settings.stiffness * (m_state.density[i] - settings.restDensity), 0.0);
    }
    

}


void SPHManager::PreCalculations()
{
    //Neighbours
    m_oCompactNSearch->neighborhood_search();
    m_vSphDiscretizations = m_oCompactNSearch->discretizations();
    int Neighbour;

    //Weights
    for (size_t i = 0; i < m_oParticleManager.GetParticlePositions()->size(); i++)
    {
        for (size_t j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
        {
            Neighbour = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;
            m_precalc.weights[i*j + j] = m_pSPHKernel.QuadricSmoothingFunctionKernel((*m_oParticleManager.GetParticlePositions())[i] - (*m_oParticleManager.GetParticlePositions())[Neighbour], settings.smoothingLength);
            m_precalc.deltaWeights[i * j + j] = m_pSPHKernel.QuadricSmoothingFunctionKernelGradient((*m_oParticleManager.GetParticlePositions())[i] - (*m_oParticleManager.GetParticlePositions())[Neighbour], settings.smoothingLength);
        }
    }

    //BoundaryPsi
    int PsiIndex;
    for (size_t i = m_oParticleManager.GetParticleContainer()->size(); i < m_oParticleManager.GetParticlePositions()->size(); i++)
    {
        PsiIndex = i - m_oParticleManager.GetParticleContainer()->size();
        for (size_t j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
        {
            Neighbour = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;
            if (Neighbour >= m_oParticleManager.GetParticleContainer()->size())
            {
                m_precalc.psi[PsiIndex] += m_precalc.weights[i * j + j];
            }
        }
        m_precalc.psi[PsiIndex] = (1 / m_precalc.psi[PsiIndex]) * settings.restDensity;
    }
}

void SPHManager::ImprovedDensityCalculation()
{
    int Neighbour;
    double Psi, V;
    float WeightSum = 0;
    for (size_t i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
    {
        m_state.density[i] = 0;
        for (size_t j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
        {
            Neighbour = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;

            //fluid neighbours
            if (Neighbour < m_oParticleManager.GetParticleContainer()->size())
            {
                m_state.density[i] += m_oParticleManager.GetParticleMass() * m_precalc.weights[i * j + j];
            }

            //boundary Neighbours
            else
            {
                m_state.density[i] += m_precalc.psi[Neighbour - m_oParticleManager.GetParticleContainer()->size()] * m_precalc.weights[i * j + j];
            }
        }
    }
}

void SPHManager::ImprovedBoundaryForceCalculation()
{
    int Neighbour;
    for (size_t i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
    {
        m_state.boundaryForce[i];
        for (size_t j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
        {
            Neighbour = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;
            if (Neighbour >= m_oParticleManager.GetParticleContainer()->size())
            {
                m_state.boundaryForce[i] -= m_oParticleManager.GetParticleMass() * m_precalc.psi[Neighbour - m_oParticleManager.GetParticleContainer()->size()]
                    * m_precalc.deltaWeights[i * j + j] * m_state.pressure[i] / (m_state.density[i] * m_state.density[i]);
            }
        }
    }
}


void SPHManager::BoundaryForceCalculation()
{
	std::vector<Eigen::Vector3d>* x = m_oParticleManager.GetParticlePositions(); //particle positions
	double dGamma;
	double q;

    auto mk = 1000.0;


	for (int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	{
        m_state.boundaryForce[i] = Eigen::Vector3d(0.0, 0.0, 0.0);
		for (int j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
		{
			int k = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;
			if (k >= m_oParticleManager.GetParticleContainer()->size())
			{
				auto diff = (*x)[i] - (*x)[k];
				auto distance = diff.norm();
				q = distance / 0.09;
				if (0.0 < q && q < 2.0 / 3.0)
				{
					dGamma = 2.0 / 3.0;
				}
				else if (2.0 / 3.0 < q && q < 1)
				{
					dGamma = 2.0 * q - (3.0 / 2.0) * q * q;
				}
				else if (1 < q && q < 2)
				{
					dGamma = 0.5 * (2 - q) * (2 - q);
				}
				else
				{
					dGamma = 0;
				}

				if (distance < DBL_EPSILON) {
					//printf("Fuuu\n");
				}

				dGamma *= 0.5 * settings.SPEED_OF_SOUND_POW / distance;
				m_state.boundaryForce[i] += (mk / (m_oParticleManager.GetParticleMass() + mk)) * dGamma * diff / distance;
			}
		}

	}
	
}