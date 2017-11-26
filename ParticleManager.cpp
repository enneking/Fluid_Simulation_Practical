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
	m_oShaderManager.initializeShader("ParticleShader.frag", "ParticleShader.vert");
	m_oShaderManager.initializeShader("BoxShader.frag", "ParticleShader.vert");

	//glEnable(GL_ARB_separate_shader_objects);
	glEnable(GL_POINT_SMOOTH);
	glPointSize(4.0f);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(10.0f);
	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glGenVertexArrays(1, &m_iVertexArrayObject);
	glGenBuffers(1, &m_iVertexBufferObject);

	glGenVertexArrays(1, &m_iVaoBox);

	glGenVertexArrays(1, &m_iVaoLine);

	SetUpBoundaryBox();

}

void ParticleManager::SetUpBoundaryBox()
{
	int n = m_iBoundariesPerFaceInOneDirection;

	//walls
	m_vBoundaryPositions[0] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxRightUpperFront[1], m_vBoxRightUpperFront[2]);
	m_vBoundaryPositions[1] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxLeftLowerBack[1], m_vBoxRightUpperFront[2]);

	m_vBoundaryPositions[2] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxRightUpperFront[1], m_vBoxRightUpperFront[2]);
	m_vBoundaryPositions[3] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxLeftLowerBack[1], m_vBoxRightUpperFront[2]);

	m_vBoundaryPositions[4] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxRightUpperFront[1], m_vBoxLeftLowerBack[2]);
	m_vBoundaryPositions[5] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxLeftLowerBack[1], m_vBoxLeftLowerBack[2]);

	m_vBoundaryPositions[6] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxRightUpperFront[1], m_vBoxLeftLowerBack[2]);
	m_vBoundaryPositions[7] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxLeftLowerBack[1], m_vBoxLeftLowerBack[2]);

	//walls
	for (int i = 0; i < 6; i++)
	{
		m_vBoundaryIndices[i * 3] = i;
		m_vBoundaryIndices[i * 3 + 1] = i + 1;
		m_vBoundaryIndices[i * 3 + 2] = i + 2;
	}
	m_vBoundaryIndices[6 * 3] = 6;
	m_vBoundaryIndices[6 * 3 + 1] = 7;
	m_vBoundaryIndices[6 * 3 + 2] = 0;

	m_vBoundaryIndices[7 * 3] = 7;
	m_vBoundaryIndices[7 * 3 + 1] = 0;
	m_vBoundaryIndices[7 * 3 + 2] = 1;

	//ceiling
	m_vBoundaryIndices[8 * 3] = 0;
	m_vBoundaryIndices[8 * 3 + 1] = 2;
	m_vBoundaryIndices[8 * 3 + 2] = 4;

	m_vBoundaryIndices[9 * 3] = 0;
	m_vBoundaryIndices[9 * 3 + 1] = 4;
	m_vBoundaryIndices[9 * 3 + 2] = 6;

	//ground
	m_vBoundaryIndices[10 * 3] = 1;
	m_vBoundaryIndices[10 * 3 + 1] = 3;
	m_vBoundaryIndices[10 * 3 + 2] = 5;

	m_vBoundaryIndices[11 * 3] = 1;
	m_vBoundaryIndices[11 * 3 + 1] = 5;
	m_vBoundaryIndices[11 * 3 + 2] = 7;	
	
	GLuint iVboBox, iElementBuffer;
	glBindVertexArray(m_iVaoBox);
	glGenBuffers(1, &iVboBox);
	glBindBuffer(GL_ARRAY_BUFFER, iVboBox);

	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vBoundaryPositions), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_vBoundaryPositions), m_vBoundaryPositions);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
	glDisableVertexAttribArray(0);

	glGenBuffers(1, &iElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_vBoundaryIndices), m_vBoundaryIndices, GL_STATIC_DRAW);

}

void ParticleManager::InitBuffers()
{
	//Particles
	glBindVertexArray(m_iVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
	glDisableVertexAttribArray(0);


	//LINE!!!!
	float line[6] = {
		2.0f, 0.0f, 0.0f,
		-2.0f, 0.0f, 0.0f
	};
	GLuint iVboLine;
	glBindVertexArray(m_iVaoLine);
	glGenBuffers(1, &iVboLine);
	glBindBuffer(GL_ARRAY_BUFFER, iVboLine);

	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
	glDisableVertexAttribArray(0);
}

void ParticleManager::AddParticle(Eigen::Vector3d fInitialPos, Eigen::Vector3d fInitialVelocity)
{
	m_vParticleContainer.emplace_back();
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

Eigen::Vector3d* ParticleManager::GetBoundaryPositions()
{
	return m_vBoundaryPositions;
}

float ParticleManager::GetParticleMass()
{
	return m_fParticleMass;
}

void ParticleManager::SetParticleMass(float value)
{
	m_fParticleMass = value;
}

void ParticleManager::DrawParticles()
{

	glUseProgram(m_oShaderManager.getProg(0));

	glm::mat4 ViewProjectionMatrix = m_pCamera->m_mProjectionMatrix * m_pCamera->m_mViewMatrix;
	GLuint ViewProjectionID = glGetUniformLocation(m_oShaderManager.getProg(0), "ViewProjection");
	glUniformMatrix4fv(ViewProjectionID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);


	//Particles
	glBindVertexArray(m_iVertexArrayObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0]);

	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_vParticlePositions.size());
	glDisableVertexAttribArray(0);

	//line
//	glBindVertexArray(m_iVaoLine);
	//glEnableVertexAttribArray(0);
//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
//	glDrawArrays(GL_LINES, 0, 6);
	//glDisableVertexAttribArray(0);

	//Box
 	glUseProgram(m_oShaderManager.getProg(1));
	ViewProjectionID = glGetUniformLocation(m_oShaderManager.getProg(1), "ViewProjection");
	glUniformMatrix4fv(ViewProjectionID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	glBindVertexArray(m_iVaoBox);

	for (int i = 0; i < 36; i++)
	{
		std::cout << m_vBoundaryIndices[i] << std::endl;
	}

	glEnableVertexAttribArray(0);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);
	glDisableVertexAttribArray(0);


}

void ParticleManager::MoveParticles(double dt)
{
	for (int i = 0; i < m_vParticleContainer.size(); i++)
	{
		m_vParticlePositions[i] += m_vParticleContainer[i].m_vVelocity * dt;
	}
}
