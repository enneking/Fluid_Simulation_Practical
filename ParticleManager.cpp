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

    // actual walls
    auto boxWidth = m_vBoxRightUpperFront.x() - m_vBoxLeftLowerBack.x();
    auto boxHeight = m_vBoxRightUpperFront.y() - m_vBoxLeftLowerBack.y();
    auto boxDepth = m_vBoxRightUpperFront.z() - m_vBoxLeftLowerBack.z();

    for (auto i = 0; i < n; i++) {  // front
        for (auto j = 0; j < n; j++) {
            m_vBoundaryPositions.push_back({ m_vBoxLeftLowerBack.x() + (boxWidth / (n - 1)) * i,  m_vBoxLeftLowerBack.x() + (boxHeight / (n - 1)) * j, m_vBoxRightUpperFront.z() });   // front
            m_vBoundaryPositions.push_back({ m_vBoxLeftLowerBack.x() + (boxWidth / (n - 1)) * i,  m_vBoxLeftLowerBack.x() + (boxHeight / (n - 1)) * j, m_vBoxLeftLowerBack.z() });     // back
            m_vBoundaryPositions.push_back({ m_vBoxRightUpperFront.x(), m_vBoxLeftLowerBack.y() + (boxHeight / (n - 1)) * i,  m_vBoxLeftLowerBack.z() + (boxDepth / (n - 1)) * j });   // right
            m_vBoundaryPositions.push_back({ m_vBoxLeftLowerBack.x(), m_vBoxLeftLowerBack.y() + (boxHeight / (n - 1)) * i,  m_vBoxLeftLowerBack.z() + (boxDepth / (n - 1)) * j });   // left
            m_vBoundaryPositions.push_back({ m_vBoxLeftLowerBack.x() + (boxWidth / (n - 1)) * i,  m_vBoxLeftLowerBack.y(), m_vBoxLeftLowerBack.z() + (boxDepth / (n - 1)) * j });   // bottom
            m_vBoundaryPositions.push_back({ m_vBoxLeftLowerBack.x() + (boxWidth / (n - 1)) * i,  m_vBoxRightUpperFront.y(), m_vBoxLeftLowerBack.z() + (boxDepth / (n - 1)) * j });   // top
        }
    }
   
	//walls
    Eigen::Vector3d boxCorners[8];

	boxCorners[0] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxRightUpperFront[1], m_vBoxRightUpperFront[2]);
	boxCorners[1] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxLeftLowerBack[1], m_vBoxRightUpperFront[2]);

	boxCorners[2] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxRightUpperFront[1], m_vBoxRightUpperFront[2]);
	boxCorners[3] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxLeftLowerBack[1], m_vBoxRightUpperFront[2]);

	boxCorners[4] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxRightUpperFront[1], m_vBoxLeftLowerBack[2]);
	boxCorners[5] = Eigen::Vector3d(m_vBoxRightUpperFront[0], m_vBoxLeftLowerBack[1], m_vBoxLeftLowerBack[2]);

	boxCorners[6] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxRightUpperFront[1], m_vBoxLeftLowerBack[2]);
	boxCorners[7] = Eigen::Vector3d(m_vBoxLeftLowerBack[0], m_vBoxLeftLowerBack[1], m_vBoxLeftLowerBack[2]);

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

	glBufferData(GL_ARRAY_BUFFER, sizeof(boxCorners), 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(boxCorners), boxCorners);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
	glDisableVertexAttribArray(0);

	glGenBuffers(1, &iElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_vBoundaryIndices), m_vBoundaryIndices, GL_STATIC_DRAW);



    GLuint iVBO;
    glGenVertexArrays(1, &Vao);
    glBindVertexArray(Vao);
    glGenBuffers(1, &iVBO);
    glBindBuffer(GL_ARRAY_BUFFER, iVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3d) * m_vBoundaryPositions.size(), &m_vBoundaryPositions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);
    glDisableVertexAttribArray(0);
}

void ParticleManager::InitBuffers()
{
	//Particles
	glBindVertexArray(m_iVertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), 0, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, 0, (void*)0);


}

ParticleManager::Particle* ParticleManager::AddParticle(Eigen::Vector3d fInitialPos, Eigen::Vector3d fInitialVelocity)
{
	m_vParticleContainer.emplace_back();
	m_vParticlePositions.push_back(fInitialPos);
	m_vParticleContainer.back().m_pPosition = &m_vParticlePositions.back();
    return &m_vParticleContainer.back();
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
	return m_vBoundaryPositions.data();
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
	glUniformMatrix4fv(glGetUniformLocation(m_oShaderManager.getProg(0), "ViewProjection"), 1, GL_FALSE, &ViewProjectionMatrix[0][0]);


	//Particles
	glBindVertexArray(m_iVertexArrayObject);

	glBindBuffer(GL_ARRAY_BUFFER, m_iVertexBufferObject);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector3d) * m_vParticlePositions.size(), &m_vParticlePositions[0]);
	glEnableVertexAttribArray(0);
	glDrawArrays(GL_POINTS, 0, (GLsizei)m_vParticlePositions.size());


    //boundary 
    glBindVertexArray(Vao);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_POINTS, 0, (GLsizei)m_vBoundaryPositions.size());

	//Box
 	glUseProgram(m_oShaderManager.getProg(1));
	glUniformMatrix4fv(glGetUniformLocation(m_oShaderManager.getProg(1), "ViewProjection"), 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	glBindVertexArray(m_iVaoBox);
	glEnableVertexAttribArray(0);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (void*)0);


}

void ParticleManager::MoveParticles(double dt)
{
	for (int i = 0; i < m_vParticleContainer.size(); i++)
	{
		m_vParticlePositions[i] += m_vParticleContainer[i].m_vVelocity * dt;
	}
}
