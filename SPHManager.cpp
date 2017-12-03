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
	m_vBoundaryForce.resize(m_oParticleManager.GetParticleContainer()->size());

	m_oCompactNSearch = std::make_unique<CompactNSearch>(m_dRadius);
	m_iDiscretizationId = m_oCompactNSearch->add_discretization(
		&(*m_oParticleManager.GetParticlePositions())[0],
		m_oParticleManager.GetParticleContainer()->size(),
		false, true);
}

void SPHManager::Update(double dt)
{
	m_pSPHKernel.Run();
	ComputeDensityAndPressure();
	BoundaryForces();

	ApplyForces(dt);

	m_oParticleManager.MoveParticles(dt);
}

void SPHManager::ApplyForces(double dt)
{

	for (unsigned int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	{
		Eigen::Vector3d acceleration(0, 0, 0);
		for (unsigned int j = 0; j < m_vSphDiscretizations[m_iDiscretizationId].n_neighbors(i); j++)
		{
			acceleration -= m_oParticleManager.GetParticleMass() * ((m_vPressure[i] / (m_vDensity[i] * m_vDensity[i])) + (m_vPressure[j] / (m_vDensity[j] * m_vDensity[j])))
				* m_pSPHKernel.QuadricSmoothingFunctionKernelGradient((*m_oParticleManager.GetParticlePositions())[i] - (*m_oParticleManager.GetParticlePositions())[j], m_dSmoothingLenght);
		}
		acceleration += m_vBoundaryForce[i];
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
			m_vDensity[i] = m_oParticleManager.GetParticleMass() * m_pSPHKernel.QuadricSmoothingFunctionKernel((*m_oParticleManager.GetParticlePositions())[i] - (*m_oParticleManager.GetParticlePositions())[j], m_dSmoothingLenght);
			m_vPressure[i] = m_dStiffness * (m_vDensity[i] - m_dRestDensity);
			if (m_vPressure[i] < 0)
			{
				m_vPressure[i] = 0;
			}
		}
	}

}

void SPHManager::BoundaryForces()
{
	std::vector<Eigen::Vector3d>* x = m_oParticleManager.GetParticlePositions(); //particle positions
	Eigen::Vector3d* b = m_oParticleManager.GetBoundaryPositions(); //boundary positions
	int bSize = m_oParticleManager.m_iBoundariesPerFaceInOneDirection * m_oParticleManager.m_iBoundariesPerFaceInOneDirection * 6;
	double dGamma;
	double q;
	for (int i = 0; i < x->size(); i++)
	{
		for (int k = 0; k < b->size(); k++)
		{
			q = (1 / m_dSmoothingLenght) * (b[k] - (*x)[i]).norm();
			if (0.0 < q && q < 2.0 / 3.0)
			{
				dGamma = 2.0 / 3.0;
			}
			else if (2.0 / 3.0 < q && q < 1)
			{
				dGamma = 2.0 * q - 3.0 * q * q / 2.0;
			}
			else if (1 < q && q < 2)
			{
				dGamma = 0.5 * (2 - q) * (2 - q);
			}
			else
			{
				dGamma = 0;
			}

			dGamma *= m_dStiffness / (b[k] - (*x)[i]).norm();

			m_vBoundaryForce[i] = 0.5f * dGamma * ((*x)[i] - b[k]) / (((*x)[i] - b[k]).norm());
		}
	}
	
}