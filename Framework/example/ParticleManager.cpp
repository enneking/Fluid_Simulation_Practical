#include "ParticleManager.h"



ParticleManager::ParticleManager()
{
}


ParticleManager::~ParticleManager()
{
}

void ParticleManager::InitBuffers()
{
	glGenBuffers(1, &m_iVertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vParticlePositions), &m_vParticlePositions[0], GL_STATIC_DRAW);
}

void ParticleManager::AddParticle(Eigen::Vector3d fInitialPos, Eigen::Vector3d fInitialVelocity)
{	
	//Particle oParticle;
	m_vParticleContainer.push_back(Particle());
	m_vParticlePositions.push_back(fInitialPos);
	m_vParticleContainer.back().m_pPosition = &m_vParticlePositions.back()[0];
}

Particle::Particle()
{
}

Particle::~Particle()
{
}

std::vector<Particle>* ParticleManager::GetParticleContainer()
{
	return &m_vParticleContainer;
}

std::vector<Eigen::Vector3d>* ParticleManager::GetParticlePositions()
{
	return &m_vParticlePositions;
}

void DrawParticles()
{


}