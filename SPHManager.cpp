#include "SPHManager.h"



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
	m_oParticleManager.InitBuffers();
	m_vDensity.resize(m_oParticleManager.GetParticleContainer()->size());
	m_vPressure.resize(m_oParticleManager.GetParticleContainer()->size());

	m_oCompactNSearch = std::make_unique<CompactNSearch>(0.1);
	m_iDiscretizationId = m_oCompactNSearch->add_discretization(
		m_oParticleManager.GetParticlePositions()->data(),
		m_oParticleManager.GetParticleContainer()->size(),
		false, true);
}

void SPHManager::Update(double dt)
{
	m_pSPHKernel.Run();
	ComputeDensityAndPressure();

	ApplyForces(dt);

	m_oParticleManager.MoveParticles(dt);
}

void SPHManager::ApplyForces(double dt)
{

	for (unsigned int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	{
		Eigen::Vector3d acceleration(0, 0, 0);
		//for (unsigned int j = 0; i < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
	//	{
			//acceleration -= m_oParticleManager.GetParticleMass() * ((m_vPressure[i] / (m_vDensity[i] * m_vDensity[i])) + (m_vPressure[j] / (m_vDensity[j] * m_vDensity[j]))) /* TODO: multiply with gradient of kernel func */;
	//	}
		acceleration[1] += m_fGravityForce;
		m_oParticleManager.GetParticleContainer()->data()[i].m_vVelocity += dt * (acceleration) * m_iSimSpeed;
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
			m_vDensity[i] = m_oParticleManager.GetParticleMass() /*TODO: multiply with kernel*/;
			m_vPressure[i] = m_dStiffness * (m_vDensity[i] - m_dRestDensity);
			if (m_vPressure[i] < 0)
			{
				m_vPressure[i] = 0;
			}
		}
	}

}