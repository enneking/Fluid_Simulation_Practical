#include "ParticleManager.h"




ParticleManager::ParticleManager()
{
}


ParticleManager::~ParticleManager()
{
}

void ParticleManager::Init(Camera* pCamera)
{
	m_pCamera = pCamera;
	m_oShaderManager.initializeShader();
	//glEnable(GL_ARB_separate_shader_objects);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5.0);
	glGenVertexArrays(1, &m_iVertexArrayObject);
	glGenBuffers(1, &m_iVertexBufferObject);
}

void ParticleManager::InitBuffers()
{


	glBindVertexArray(m_iVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0]);
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

float ParticleManager::GetParticleMass()
{
	return m_fParticleMass;
}

void ParticleManager::SetParticleMass(float value)
{
	m_fParticleMass = value;
}

void ParticleManager::DrawParticles(  )
{
	glUseProgram(m_oShaderManager.getProg());

	glm::mat4 ViewProjectionMatrix = m_pCamera->m_mProjectionMatrix * m_pCamera->m_mViewMatrix;

	GLuint ViewProjectionID = glGetUniformLocation(m_oShaderManager.getProg(), "ViewProjection");
	glUniformMatrix4fv(ViewProjectionID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
	
	glBindVertexArray(m_iVertexArrayObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0]);

	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_vParticlePositions.size());
	glDisableVertexAttribArray(0);
}

void ParticleManager::MoveParticles(double dt)
{
	for (int i = 0; i < m_vParticleContainer.size(); i++)
	{
		m_vParticlePositions[i] += m_vParticleContainer[i].m_vVelocity * dt;
	}
	
}