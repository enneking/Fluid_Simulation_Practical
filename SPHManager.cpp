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
}

void SPHManager::Update(double dt)
{
    m_pSPHKernel.Run();

	CalculateGravitation();

	m_oParticleManager.MoveParticles(dt);
}

void SPHManager::CalculateGravitation()
{
	//for (int i = 0; i < m_oParticleManager.GetParticleContainer()->size(); i++)
	//{
	//	m_oParticleManager.GetParticleContainer()->data()[i].m_vVelocity[1] += m_fGravityForce;
	//}
}