#include "SPHManager.h"
#include "imgui.h"


SPHManager::SPHManager()
{
}


SPHManager::~SPHManager()
{
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
	m_vDensity.resize(m_oParticleManager.GetParticleContainer()->size());
	m_vPressure.resize(m_oParticleManager.GetParticleContainer()->size());
	m_vBoundaryForce.resize(m_oParticleManager.GetParticleContainer()->size());

	m_oCompactNSearch = std::make_unique<CompactNSearch>(m_dRadius);
	m_iDiscretizationId = m_oCompactNSearch->add_discretization(
		&(*m_oParticleManager.GetParticlePositions())[0],
		m_oParticleManager.GetParticleContainer()->size(),
		false, true);
}

void SPHManager::Update(double dt)
{
	if (m_bRun == false)
	{
		return;
	}
	dt *= m_iSimSpeed;

	ComputeDensityAndPressure();

    BoundaryForces();
    ApplyForces(dt);

    ApplyViscosity();

	m_oParticleManager.MoveParticles(dt);

}


void SPHManager::ApplyViscosity()
{
    constexpr double epsilon = 0.05;
    auto particles = m_oParticleManager.GetParticleContainer()->data();
    for (auto pidx = 0; pidx < m_oParticleManager.GetParticleContainer()->size(); ++pidx) {
        auto& p = particles[pidx];
        auto vPrime = p.m_vVelocity;
        for (unsigned int didx = 0; didx < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(pidx); didx++)
        {
            int j = m_vSphDiscretizations[m_iDiscretizationId].neighbor(pidx, didx).index;
            vPrime += (m_oParticleManager.GetParticleMass() / m_vDensity[j]) * (particles[j].m_vVelocity - p.m_vVelocity) * m_pSPHKernel.QuadricSmoothingFunctionKernel((*m_oParticleManager.GetParticlePositions())[pidx] - (*m_oParticleManager.GetParticlePositions())[j], m_dSmoothingLength);
        }
        vPrime *= epsilon;
        p.m_vVelocity = vPrime;
    }
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
        }, m_vBoundaryForce.data(), m_vBoundaryForce.size());

        if (ImGui::TreeNode("Settings")) {

            float gravity = (float)m_fGravityForce;
            float stiffness = (float)m_dStiffness;
            float simSpeed = (float)m_iSimSpeed;
            float restDens = (float)m_dRestDensity;
            float mass = (float)m_oParticleManager.GetParticleMass();
            float radius = (float)m_dRadius;
            float smoothingLength = (float)m_dSmoothingLength;

            ImGui::DragFloat("Gravity", &gravity, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat("Stiffness", &stiffness, 1.0f);
            ImGui::DragFloat("Simulation Speed", &simSpeed, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat("Rest Density", &restDens, 0.01f);
            ImGui::DragFloat("Mass", &mass, 0.01f);
            ImGui::DragFloat("Kernel Radius", &radius, 0.01);
            ImGui::DragFloat("Smoothing Length", &smoothingLength, 0.01f, 0.1f, 1.0f);

            m_fGravityForce = (double)gravity;
            m_dStiffness = (double)stiffness;
            m_iSimSpeed = (double)simSpeed;
            m_dRestDensity = (double)restDens;
            m_oParticleManager.SetParticleMass((double)mass);
            m_dSmoothingLength = smoothingLength;

            m_dRadius = (double)radius;
            m_oCompactNSearch->set_radius(m_dRadius);

            ImGui::TreePop();

        }

    } ImGui::End();
}


void SPHManager::ApplyForces(double dt)
{

	for (unsigned int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	{
		Eigen::Vector3d acceleration(0, 0, 0);
		for (unsigned int j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
		{
			int index = m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index;
			acceleration += m_oParticleManager.GetParticleMass() * ((m_vPressure[i] / (m_vDensity[i] * m_vDensity[i])) + (m_vPressure[index] / (m_vDensity[index] * m_vDensity[index])))
				* m_pSPHKernel.QuadricSmoothingFunctionKernelGradient((*m_oParticleManager.GetParticlePositions())[i] - (*m_oParticleManager.GetParticlePositions())[index], m_dSmoothingLength);
		}
        acceleration *= -1.0;
		acceleration += (1.0 / m_oParticleManager.GetParticleMass()) * m_vBoundaryForce[i];
		acceleration[1] += m_fGravityForce;
		m_oParticleManager.GetParticleContainer()->data()[i].m_vVelocity += dt * (acceleration);
	}
}

void SPHManager::ComputeDensityAndPressure()
{
	m_oCompactNSearch->neighborhood_search();
	m_vSphDiscretizations = m_oCompactNSearch->discretizations();

	for (int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	{
		for (unsigned int j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
		{
            m_vDensity[i] += m_oParticleManager.GetParticleMass() * m_pSPHKernel.QuadricSmoothingFunctionKernel((*m_oParticleManager.GetParticlePositions())[i] 
				- (*m_oParticleManager.GetParticlePositions())[m_vSphDiscretizations[m_iDiscretizationId].neighbor(i, j).index], m_dSmoothingLength);
		
		}
		m_vPressure[i] = glm::max(m_dStiffness * (m_vDensity[i] - m_dRestDensity), 0.0);
	}

}

void SPHManager::BoundaryForces()
{
	std::vector<Eigen::Vector3d>* x = m_oParticleManager.GetParticlePositions(); //particle positions
	Eigen::Vector3d* b = m_oParticleManager.GetBoundaryPositions(); //boundary positions
	int bSize = m_oParticleManager.m_iBoundariesPerFaceInOneDirection * m_oParticleManager.m_iBoundariesPerFaceInOneDirection * 6;
	double dGamma;
	double q;

    auto mk = 1000.0;


	for (int i = 0; i < x->size(); i++)
	{
        m_vBoundaryForce[i] = Eigen::Vector3d(0.0, 0.0, 0.0);
		for (int k = 0; k < bSize; k++)
		{
            auto diff = (*x)[i] - b[k];
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
                printf("Fuuu\n");
            }

			dGamma *= 0.5 * SPEED_OF_SOUND_POW / distance;
			m_vBoundaryForce[i] += (mk / (m_oParticleManager.GetParticleMass() + mk)) * dGamma * diff / distance;
		}

        //m_vBoundaryForce[i] = Eigen::Vector3d(0.0f, 1.0f, 0.0f);
	}
	
}