#include "ParticleManager.h"



ParticleManager::ParticleManager()
{
}


ParticleManager::~ParticleManager()
{
}

void ParticleManager::Init()
{
	m_oShaderManager.initializeShader();
	//glEnable(GL_ARB_separate_shader_objects);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(10.0);
	glGenVertexArrays(1, &m_iVertexArrayObject);
	glGenBuffers(1, &m_iVertexBufferObject);
}

void ParticleManager::InitBuffers()
{


	glBindVertexArray(m_iVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
}

void ParticleManager::AddParticle(Eigen::Vector3d fInitialPos, Eigen::Vector3d fInitialVelocity)
{	
	//Particle oParticle;
	m_vParticleContainer.push_back(Particle());
	m_vParticlePositions.push_back(fInitialPos);
	m_vParticleContainer.back().m_pPosition = &m_vParticlePositions.back();
}

ParticleManager::Particle::Particle()
{
}

ParticleManager::Particle::~Particle()
{
}

std::vector<ParticleManager::Particle>* ParticleManager::GetParticleContainer()
{
	return &m_vParticleContainer;
}

std::vector<Eigen::Vector3d>* ParticleManager::GetParticlePositions()
{
	return &m_vParticlePositions;
}

void ParticleManager::DrawParticles()
{
	glUseProgram(m_oShaderManager.getProg());
	glBindVertexArray(m_iVertexArrayObject);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, m_vParticlePositions.size());
	glDisableVertexAttribArray(0);
}
