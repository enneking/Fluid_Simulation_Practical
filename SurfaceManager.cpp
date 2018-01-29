#include "SurfaceManager.h"
#include <fstream>

void SurfaceManager::Init(SPHManager* pSPHManager)
{
	m_pParticleManager = pSPHManager->GetParticleManager();
	m_density = &pSPHManager->state.density;
	m_pKernel = pSPHManager->m_pSPHKernel;
	auto UpperRightFrontVec = m_pParticleManager->m_vBoxRightUpperFront;
	m_LowerLeftBackVec = m_pParticleManager->m_vBoxLeftLowerBack;


	m_Mesh.i_max = 6+(size_t)(((UpperRightFrontVec)[0] - (m_LowerLeftBackVec)[0]) / m_CubeWidth);
	m_Mesh.j_max = 6+(size_t)(((UpperRightFrontVec)[1] - (m_LowerLeftBackVec)[1]) / m_CubeWidth);
	m_Mesh.k_max = 6+(size_t)(((UpperRightFrontVec)[2] - (m_LowerLeftBackVec)[2]) / m_CubeWidth);

	m_Mesh.Pos.reserve((m_Mesh.i_max) * (m_Mesh.j_max) * (m_Mesh.k_max));
	m_VoxelWeights.resize((m_Mesh.i_max) * (m_Mesh.j_max) * (m_Mesh.k_max));

	for (int i = 0; i < m_Mesh.i_max; i++)
	{
		for (int j = 0; j < m_Mesh.j_max; j++)
		{
			for (int k = 0; k < m_Mesh.k_max; k++)
			{
				m_Mesh.Pos.emplace_back((m_LowerLeftBackVec)[0] + (i-3) * m_CubeWidth, (m_LowerLeftBackVec)[1] + (j-3) * m_CubeWidth, (m_LowerLeftBackVec)[1] + (k-3) * m_CubeWidth);
			}
		}
	}

	m_ShaderManager.initializeShader("SurfaceShader.frag", "SurfaceShader.vert");

	glGenVertexArrays(1, &m_iVertexArrayObject);
	glBindVertexArray(m_iVertexArrayObject);

	GLuint iVertexBufferObject;
	glGenBuffers(1, &iVertexBufferObject);

	glBindBuffer(GL_ARRAY_BUFFER, iVertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_SurfaceVertices.size(), 0, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat) * 3));
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	//m_pCompactNSearch = std::make_unique<CompactNSearch>(0.1);
	//m_fluidDiscretizationId = m_pCompactNSearch->add_discretization(
	//	&(*m_pParticleManager->GetParticlePositions())[0],
	//	m_pParticleManager->GetParticleContainer()->size(),
	//	true, false);
	//m_surfaceDiscretizationId = m_pCompactNSearch->add_discretization(
	//	&m_Mesh.Pos[0],
	//	m_Mesh.Pos.size(),
	//	false, true);

	////m_pCompactNSearch->neighborhood_search();
	
}

void SurfaceManager::Draw()
{
	glm::vec3 *CameraPos = m_pParticleManager->GetCamera()->GetPosition();
	glUniform3fv(glGetUniformLocation(m_ShaderManager.getProg(0), "view_pos"), 1, &(*CameraPos)[0]);

	glm::mat4 ProjectionMatrix = m_pParticleManager->GetCamera()->m_mProjectionMatrix;
	glUniformMatrix4fv(glGetUniformLocation(m_ShaderManager.getProg(0), "uProjectionMatrix"), 1, GL_FALSE, &ProjectionMatrix[0][0]);

	glm::mat4 ViewMatrix = m_pParticleManager->GetCamera()->m_mViewMatrix;
	glUniformMatrix4fv(glGetUniformLocation(m_ShaderManager.getProg(0), "uViewMatrix"), 1, GL_FALSE, &ViewMatrix[0][0]);

	glEnable(GL_DEPTH_TEST);
	glUseProgram(m_ShaderManager.getProg(0));

	glBindVertexArray(m_iVertexArrayObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * m_SurfaceVertices.size(), &m_SurfaceVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat) * 3));
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)m_SurfaceVertices.size() / 3);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisable(GL_DEPTH_TEST);
}


void SurfaceManager::CreateSurface()
{
	std::fill(m_VoxelWeights.begin(), m_VoxelWeights.end(), 0);
	for (size_t i = 0; i < m_pParticleManager->GetParticleContainer()->size(); i++)
	{
		int x_i = (int)(3+((*m_pParticleManager->GetParticlePositions())[i][0] - m_LowerLeftBackVec[0]) / m_CubeWidth);
		int x_j = (int)(3+((*m_pParticleManager->GetParticlePositions())[i][1] - m_LowerLeftBackVec[1]) / m_CubeWidth);
		int x_k = (int)(3+((*m_pParticleManager->GetParticlePositions())[i][2] - m_LowerLeftBackVec[2]) / m_CubeWidth);

		volatile int i_0 = x_i * (int)(m_Mesh.j_max) * (int)(m_Mesh.k_max) + x_j * (int)(m_Mesh.k_max) + x_k;
		int i_1 = (x_i + 1) * (int)m_Mesh.j_max *(int)m_Mesh.k_max + x_j * (int)m_Mesh.k_max + x_k;
		int i_2 = (x_i + 1) * (int)m_Mesh.j_max * (int)m_Mesh.k_max + x_j * (int)m_Mesh.k_max + (x_k + 1);
		int i_3 = x_i * (int)m_Mesh.j_max * (int)m_Mesh.k_max + x_j * (int)m_Mesh.k_max + (x_k + 1);
		int i_4 = x_i * (int)m_Mesh.j_max * (int)m_Mesh.k_max + (x_j + 1) * (int)m_Mesh.k_max + x_k;
		int i_5 = (x_i + 1) *(int)m_Mesh.j_max * (int)m_Mesh.k_max + (x_j + 1) * (int)m_Mesh.k_max + x_k;
		int i_6 = (x_i + 1) * (int)m_Mesh.j_max * (int)m_Mesh.k_max + (x_j + 1) * (int)m_Mesh.k_max + (x_k + 1);
		volatile int i_7 = x_i * (int)m_Mesh.j_max * (int)m_Mesh.k_max + (x_j + 1) * (int)m_Mesh.k_max + (x_k + 1);
		//std::cout << "i " << i << std::endl;
		//std::cout << "xi " << x_i << std::endl;
		//std::cout << "xj " << x_j << std::endl;
		//std::cout << "xk " << x_k << std::endl;
		//std::cout << "mesh size " << m_VoxelWeights.size() << std::endl;
		//std::cout << "particle size " << m_pParticleManager->GetParticlePositions()->size() << std::endl;
		//std::cout << "density size " << m_density->size() << std::endl;
		//std::cout << "i0 " << i_0 << std::endl;
		//std::cout << "i7 " << i_7 << std::endl;

		//std::cout << "----------------\n";

		if (i_6 < m_VoxelWeights.size() - (int)(3 * (m_Mesh.j_max) * (m_Mesh.k_max) + 3 * (m_Mesh.k_max) + 3) && i_0 > (int)(3 * (m_Mesh.j_max) * (m_Mesh.k_max) + 3 * (m_Mesh.k_max) + 3))
		{

			m_VoxelWeights[i_0] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_0] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_1] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_1] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_2] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_2] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_3] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_3] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_4] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_4] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_5] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_5] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_6] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_6] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
			m_VoxelWeights[i_7] += (float)(m_pKernel->Evaluate(m_Mesh.Pos[i_7] - (*m_pParticleManager->GetParticlePositions())[i]) * m_pParticleManager->GetParticleMass() / ((*m_density)[i]));
		}

	}


	//for (size_t i = 0; i < m_Mesh.Pos.size(); i++)
	//{
	//	auto num_neighbors = discretization.n_neighbors((unsigned int)i);
	//	double sum_weight = 0.0f;
	//	for(size_t j = 0; j < num_neighbors; j++)
	//	{
	//		if (discretization.neighbor((unsigned int)i, (unsigned int)j).object_id == m_fluidDiscretizationId)
	//		{
	//			auto k = discretization.neighbor((unsigned int)i, (unsigned int)j).index;
	//			if ((*m_density)[k] > 0.0f)
	//			{
	//				sum_weight += m_pKernel->Evaluate(m_Mesh.Pos[i] - (*m_pParticleManager->GetParticlePositions())[k]) * m_pParticleManager->GetParticleMass() / ((*m_density)[k]);
	//			}
	//		}
	//	}
	//	m_VoxelWeights[i] = (float)sum_weight;
	//}
	uint8_t cellmarks;
	m_SurfaceVertices.clear();
	for (size_t i = 0; i < (m_Mesh.i_max - 2); i++)
	{

		for (size_t j = 0; j < (m_Mesh.j_max - 2); j++)
		{
			for (size_t k = 0; k < (m_Mesh.k_max - 2); k++)
			{
				size_t i_full = i * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + k;
				size_t i_1 = (i + 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + k;
				size_t i_2 = (i + 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1);
				size_t i_3 = i * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1);
				size_t i_4 = i * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + k;
				size_t i_5 = (i + 1) *m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + k;
				size_t i_6 = (i + 1) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1);
				size_t i_7 = i * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1);


				cellmarks  = (m_VoxelWeights[i_full] > 0);

				cellmarks |= (m_VoxelWeights[i_1] > m_kSurfaceConst) << 1;

				cellmarks |= (m_VoxelWeights[i_2] > m_kSurfaceConst) << 2;

				cellmarks |= (m_VoxelWeights[i_3] > m_kSurfaceConst) << 3;

				cellmarks |= (m_VoxelWeights[i_4] > m_kSurfaceConst) << 4;

				cellmarks |= (m_VoxelWeights[i_5] > m_kSurfaceConst) << 5;
				
				cellmarks |= (m_VoxelWeights[i_6] > m_kSurfaceConst) << 6;

				cellmarks |= (m_VoxelWeights[i_7] > m_kSurfaceConst) << 7;

				for (size_t l = 0; marching_cubes_lut[cellmarks][l] != -1; l++)
				{

					float x0, y0, z0, x1, y1, z1;
					float offset;
					switch (marching_cubes_lut[cellmarks][l]) {		

					case 0:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_full]) / (m_VoxelWeights[i_1] - m_VoxelWeights[i_full])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2]);

						x0 = (m_VoxelWeights[i_1] - m_VoxelWeights[(i - 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + k]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_4] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_3] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + k] - m_VoxelWeights[i_full]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i_5] - m_VoxelWeights[(i+1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i_2] - m_VoxelWeights[(i+1) * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						break;

					case 1:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_1]) / (m_VoxelWeights[i_2] - m_VoxelWeights[i_1])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + offset);


						x0 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + k] - m_VoxelWeights[i_full]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_5] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_2] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k+1)] - m_VoxelWeights[i_3]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + (k + 1)]) / m_CubeWidth;
						z1 = (m_VoxelWeights[(i + 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_1]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);

						break;

					case 2:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_3]) / (m_VoxelWeights[i_2] - m_VoxelWeights[i_3])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + m_CubeWidth);

						x0 = (m_VoxelWeights[i_2] - m_VoxelWeights[(i - 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_7] - m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j - 1) * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_full]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_3]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + (k + 1)]) / m_CubeWidth;
						z1 = (m_VoxelWeights[(i + 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_1]) / m_CubeWidth;
					
						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);

						break;

					case 3:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_full]) / (m_VoxelWeights[i_3] - m_VoxelWeights[i_full])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + offset);

						x0 = (m_VoxelWeights[i_1] - m_VoxelWeights[(i - 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + k]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_4] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_3] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[i_2] - m_VoxelWeights[(i - 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i_7] - m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j - 1) * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_full]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);

						break;

					case 4:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_4]) / (m_VoxelWeights[i_5] - m_VoxelWeights[i_4])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2]);

						x0 = (m_VoxelWeights[i_5] - m_VoxelWeights[(i - 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + k] - m_VoxelWeights[i_full]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_7] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j+1) * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + (j+1) * m_Mesh.k_max + k] - m_VoxelWeights[i_4]) / m_CubeWidth;
						y1 = (m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + k] - m_VoxelWeights[i_1]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j+1) * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);

						break;
						
					case 5:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_5]) / (m_VoxelWeights[i_6] - m_VoxelWeights[i_5])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + offset);

						x0 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + k] - m_VoxelWeights[i_4]) / m_CubeWidth;
						y0 = (m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + k] - m_VoxelWeights[i_1]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_7]) / m_CubeWidth;
						y1 = (m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + (k + 1)] - m_VoxelWeights[i_2]) / m_CubeWidth;
						z1 = (m_VoxelWeights[(i + 1) * m_Mesh.j_max * m_Mesh.k_max + (j+1) * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_5]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						
						break;
						
						
					case 6:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_7]) / (m_VoxelWeights[i_6] - m_VoxelWeights[i_7])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + m_CubeWidth);

						x0 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i - 1) * m_Mesh.j_max * m_Mesh.k_max + (j+1) * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j + 2) * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_3]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j+1) * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_4]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_7]) / m_CubeWidth;
						y1 = (m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + (k + 1)] - m_VoxelWeights[i_2]) / m_CubeWidth;
						z1 = (m_VoxelWeights[(i + 1) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_5]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);

						break;

					case 7:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_4]) / (m_VoxelWeights[i_7] - m_VoxelWeights[i_4])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + offset);

						x0 = (m_VoxelWeights[i_5] - m_VoxelWeights[(i - 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + k] - m_VoxelWeights[i_full]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_7] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i - 1) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j + 2) * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_3]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_4]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						break;	

					case 8:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_full]) / (m_VoxelWeights[i_4] - m_VoxelWeights[i_full])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2]);

						x0 = (m_VoxelWeights[i_1] - m_VoxelWeights[(i - 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + k]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_4] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i_3] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[i_5] - m_VoxelWeights[(i - 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + k] - m_VoxelWeights[i_full]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i_7] - m_VoxelWeights[i * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						break;

					case 9:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_1]) / (m_VoxelWeights[i_5] - m_VoxelWeights[i_1])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2]);

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + k] - m_VoxelWeights[i_full]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i_5] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + k]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i_2] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + j * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + k] - m_VoxelWeights[i_4]) / m_CubeWidth;
						y1 = (m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + k] - m_VoxelWeights[i_1]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 1) * (m_Mesh.k_max) + (k - 1)]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						break;

					case 10:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_2]) / (m_VoxelWeights[i_6] - m_VoxelWeights[i_2])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0] + m_CubeWidth);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + m_CubeWidth);

						x0 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_3]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j - 1) * (m_Mesh.k_max) + (k + 1)]) / m_CubeWidth;
						z0 = (m_VoxelWeights[(i + 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_1]) / m_CubeWidth;

						x1 = (m_VoxelWeights[(i + 2) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_7]) / m_CubeWidth;
						y1 = (m_VoxelWeights[(i + 1) * (m_Mesh.j_max) * (m_Mesh.k_max) + (j + 2) * (m_Mesh.k_max) + (k + 1)] - m_VoxelWeights[i_2]) / m_CubeWidth;
						z1 = (m_VoxelWeights[(i + 1) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_5]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						break;

					case 11:
						offset = (float)((m_kSurfaceConst - m_VoxelWeights[i_3]) / (m_VoxelWeights[i_7] - m_VoxelWeights[i_3])) * m_CubeWidth;

						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][0]);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][1] + offset);
						m_SurfaceVertices.emplace_back(m_Mesh.Pos[i_full][2] + m_CubeWidth);
						
						x0 = (m_VoxelWeights[i_2] - m_VoxelWeights[(i - 1) * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						y0 = (m_VoxelWeights[i_7] - m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j - 1) * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						z0 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + j * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_full]) / m_CubeWidth;

						x1 = (m_VoxelWeights[i_6] - m_VoxelWeights[(i - 1) * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 1)]) / m_CubeWidth;
						y1 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j + 2) * m_Mesh.k_max + (k + 1)] - m_VoxelWeights[i_3]) / m_CubeWidth;
						z1 = (m_VoxelWeights[i * m_Mesh.j_max * m_Mesh.k_max + (j + 1) * m_Mesh.k_max + (k + 2)] - m_VoxelWeights[i_4]) / m_CubeWidth;

						m_SurfaceVertices.emplace_back(x0 + offset * (x1 - x0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(y0 + offset * (y1 - y0) / m_CubeWidth);
						m_SurfaceVertices.emplace_back(z0 + offset * (z1 - z0) / m_CubeWidth);
						break;

					default:
						break;
					}
				}
			}
		}
	}
	

}




bool SurfaceManager::SaveVertexPosToFile(std::ofstream& file)
{
	std::vector<char> buf;

	buf.resize(sizeof(GLfloat) * m_SurfaceVertices.size());
	memcpy(buf.data(), m_SurfaceVertices.data(), buf.size());

	auto res = buf.size() > 0 && file.is_open();
	if (res)
	{
		uint64_t bufSize = (uint64_t)m_SurfaceVertices.size();
		file.write((char*)&bufSize, sizeof(uint64_t));
		file.write(buf.data(), buf.size());
	}

	return res;
}

bool SurfaceManager::LoadVertexPosFromFile(std::ifstream& file)
{

	if (file.peek() == EOF) { return false; }
	uint64_t bufSize = 0;
	file.read((char*)&bufSize, sizeof(uint64_t));
	m_SurfaceVertices.resize(bufSize);
	file.read((char*)m_SurfaceVertices.data(), bufSize * sizeof(GLfloat));
	return true;
}


int8_t const SurfaceManager::marching_cubes_lut[256][16] = {
	/*   0:                          */{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   1: 0,                       */{ 0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   2:    1,                    */{ 0,  1,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   3: 0, 1,                    */{ 1,  8,  3,  9,  8,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   4:       2,                 */{ 1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   5: 0,    2,                 */{ 0,  8,  3,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   6:    1, 2,                 */{ 9,  2, 10,  0,  2,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   7: 0, 1, 2,                 */{ 2,  8,  3,  2, 10,  8, 10,  9,  8, -1, -1, -1, -1, -1, -1, -1 },
	/*   8:          3,              */{ 3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*   9: 0,       3,              */{ 0, 11,  2,  8, 11,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  10:    1,    3,              */{ 1,  9,  0,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  11: 0, 1,    3,              */{ 1, 11,  2,  1,  9, 11,  9,  8, 11, -1, -1, -1, -1, -1, -1, -1 },
	/*  12:       2, 3,              */{ 3, 10,  1, 11, 10,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  13: 0,    2, 3,              */{ 0, 10,  1,  0,  8, 10,  8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
	/*  14:    1, 2, 3,              */{ 3,  9,  0,  3, 11,  9, 11, 10,  9, -1, -1, -1, -1, -1, -1, -1 },
	/*  15: 0, 1, 2, 3,              */{ 9,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  16:             4,           */{ 4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  17: 0,          4,           */{ 4,  3,  0,  7,  3,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  18:    1,       4,           */{ 0,  1,  9,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  19: 0, 1,       4,           */{ 4,  1,  9,  4,  7,  1,  7,  3,  1, -1, -1, -1, -1, -1, -1, -1 },
	/*  20:       2,    4,           */{ 1,  2, 10,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  21: 0,    2,    4,           */{ 3,  4,  7,  3,  0,  4,  1,  2, 10, -1, -1, -1, -1, -1, -1, -1 },
	/*  22:    1, 2,    4,           */{ 9,  2, 10,  9,  0,  2,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1 },
	/*  23: 0, 1, 2,    4,           */{ 2, 10,  9,  2,  9,  7,  2,  7,  3,  7,  9,  4, -1, -1, -1, -1 },
	/*  24:          3, 4,           */{ 8,  4,  7,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  25: 0,       3, 4,           */{ 11,  4,  7, 11,  2,  4,  2,  0,  4, -1, -1, -1, -1, -1, -1, -1 },
	/*  26:    1,    3, 4,           */{ 9,  0,  1,  8,  4,  7,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1 },
	/*  27: 0, 1,    3, 4,           */{ 4,  7, 11,  9,  4, 11,  9, 11,  2,  9,  2,  1, -1, -1, -1, -1 },
	/*  28:       2, 3, 4,           */{ 3, 10,  1,  3, 11, 10,  7,  8,  4, -1, -1, -1, -1, -1, -1, -1 },
	/*  29: 0,    2, 3, 4,           */{ 1, 11, 10,  1,  4, 11,  1,  0,  4,  7, 11,  4, -1, -1, -1, -1 },
	/*  30:    1, 2, 3, 4,           */{ 4,  7,  8,  9,  0, 11,  9, 11, 10, 11,  0,  3, -1, -1, -1, -1 },
	/*  31: 0, 1, 2, 3, 4,           */{ 4,  7, 11,  4, 11,  9,  9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
	/*  32:                5,        */{ 9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  33: 0,             5,        */{ 9,  5,  4,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  34:    1,          5,        */{ 0,  5,  4,  1,  5,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  35: 0, 1,          5,        */{ 8,  5,  4,  8,  3,  5,  3,  1,  5, -1, -1, -1, -1, -1, -1, -1 },
	/*  36:       2,       5,        */{ 1,  2, 10,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  37: 0,    2,       5,        */{ 3,  0,  8,  1,  2, 10,  4,  9,  5, -1, -1, -1, -1, -1, -1, -1 },
	/*  38:    1, 2,       5,        */{ 5,  2, 10,  5,  4,  2,  4,  0,  2, -1, -1, -1, -1, -1, -1, -1 },
	/*  39: 0, 1, 2,       5,        */{ 2, 10,  5,  3,  2,  5,  3,  5,  4,  3,  4,  8, -1, -1, -1, -1 },
	/*  40:          3,    5,        */{ 9,  5,  4,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  41: 0,       3,    5,        */{ 0, 11,  2,  0,  8, 11,  4,  9,  5, -1, -1, -1, -1, -1, -1, -1 },
	/*  42:    1,    3,    5,        */{ 0,  5,  4,  0,  1,  5,  2,  3, 11, -1, -1, -1, -1, -1, -1, -1 },
	/*  43: 0, 1,    3,    5,        */{ 2,  1,  5,  2,  5,  8,  2,  8, 11,  4,  8,  5, -1, -1, -1, -1 },
	/*  44:       2, 3,    5,        */{ 10,  3, 11, 10,  1,  3,  9,  5,  4, -1, -1, -1, -1, -1, -1, -1 },
	/*  45: 0,    2, 3,    5,        */{ 4,  9,  5,  0,  8,  1,  8, 10,  1,  8, 11, 10, -1, -1, -1, -1 },
	/*  46:    1, 2, 3,    5,        */{ 5,  4,  0,  5,  0, 11,  5, 11, 10, 11,  0,  3, -1, -1, -1, -1 },
	/*  47: 0, 1, 2, 3,    5,        */{ 5,  4,  8,  5,  8, 10, 10,  8, 11, -1, -1, -1, -1, -1, -1, -1 },
	/*  48:             4, 5,        */{ 9,  7,  8,  5,  7,  9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  49: 0,          4, 5,        */{ 9,  3,  0,  9,  5,  3,  5,  7,  3, -1, -1, -1, -1, -1, -1, -1 },
	/*  50:    1,       4, 5,        */{ 0,  7,  8,  0,  1,  7,  1,  5,  7, -1, -1, -1, -1, -1, -1, -1 },
	/*  51: 0, 1,       4, 5,        */{ 1,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  52:       2,    4, 5,        */{ 9,  7,  8,  9,  5,  7, 10,  1,  2, -1, -1, -1, -1, -1, -1, -1 },
	/*  53: 0,    2,    4, 5,        */{ 10,  1,  2,  9,  5,  0,  5,  3,  0,  5,  7,  3, -1, -1, -1, -1 },
	/*  54:    1, 2,    4, 5,        */{ 8,  0,  2,  8,  2,  5,  8,  5,  7, 10,  5,  2, -1, -1, -1, -1 },
	/*  55: 0, 1, 2,    4, 5,        */{ 2, 10,  5,  2,  5,  3,  3,  5,  7, -1, -1, -1, -1, -1, -1, -1 },
	/*  56:          3, 4, 5,        */{ 7,  9,  5,  7,  8,  9,  3, 11,  2, -1, -1, -1, -1, -1, -1, -1 },
	/*  57: 0,       3, 4, 5,        */{ 9,  5,  7,  9,  7,  2,  9,  2,  0,  2,  7, 11, -1, -1, -1, -1 },
	/*  58:    1,    3, 4, 5,        */{ 2,  3, 11,  0,  1,  8,  1,  7,  8,  1,  5,  7, -1, -1, -1, -1 },
	/*  59: 0, 1,    3, 4, 5,        */{ 11,  2,  1, 11,  1,  7,  7,  1,  5, -1, -1, -1, -1, -1, -1, -1 },
	/*  60:       2, 3, 4, 5,        */{ 9,  5,  8,  8,  5,  7, 10,  1,  3, 10,  3, 11, -1, -1, -1, -1 },
	/*  61: 0,    2, 3, 4, 5,        */{ 5,  7,  0,  5,  0,  9,  7, 11,  0,  1,  0, 10, 11, 10,  0, -1 },
	/*  62:    1, 2, 3, 4, 5,        */{ 11, 10,  0, 11,  0,  3, 10,  5,  0,  8,  0,  7,  5,  7,  0, -1 },
	/*  63: 0, 1, 2, 3, 4, 5,        */{ 11, 10,  5,  7, 11,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  64:                   6,     */{ 10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  65: 0,                6,     */{ 0,  8,  3,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  66:    1,             6,     */{ 9,  0,  1,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  67: 0, 1,             6,     */{ 1,  8,  3,  1,  9,  8,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1 },
	/*  68:       2,          6,     */{ 1,  6,  5,  2,  6,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  69: 0,    2,          6,     */{ 1,  6,  5,  1,  2,  6,  3,  0,  8, -1, -1, -1, -1, -1, -1, -1 },
	/*  70:    1, 2,          6,     */{ 9,  6,  5,  9,  0,  6,  0,  2,  6, -1, -1, -1, -1, -1, -1, -1 },
	/*  71: 0, 1, 2,          6,     */{ 5,  9,  8,  5,  8,  2,  5,  2,  6,  3,  2,  8, -1, -1, -1, -1 },
	/*  72:          3,       6,     */{ 2,  3, 11, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  73: 0,       3,       6,     */{ 11,  0,  8, 11,  2,  0, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1 },
	/*  74:    1,    3,       6,     */{ 0,  1,  9,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1, -1, -1, -1 },
	/*  75: 0, 1,    3,       6,     */{ 5, 10,  6,  1,  9,  2,  9, 11,  2,  9,  8, 11, -1, -1, -1, -1 },
	/*  76:       2, 3,       6,     */{ 6,  3, 11,  6,  5,  3,  5,  1,  3, -1, -1, -1, -1, -1, -1, -1 },
	/*  77: 0,    2, 3,       6,     */{ 0,  8, 11,  0, 11,  5,  0,  5,  1,  5, 11,  6, -1, -1, -1, -1 },
	/*  78:    1, 2, 3,       6,     */{ 3, 11,  6,  0,  3,  6,  0,  6,  5,  0,  5,  9, -1, -1, -1, -1 },
	/*  79: 0, 1, 2, 3,       6,     */{ 6,  5,  9,  6,  9, 11, 11,  9,  8, -1, -1, -1, -1, -1, -1, -1 },
	/*  80:             4,    6,     */{ 5, 10,  6,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  81: 0,          4,    6,     */{ 4,  3,  0,  4,  7,  3,  6,  5, 10, -1, -1, -1, -1, -1, -1, -1 },
	/*  82:    1,       4,    6,     */{ 1,  9,  0,  5, 10,  6,  8,  4,  7, -1, -1, -1, -1, -1, -1, -1 },
	/*  83: 0, 1,       4,    6,     */{ 10,  6,  5,  1,  9,  7,  1,  7,  3,  7,  9,  4, -1, -1, -1, -1 },
	/*  84:       2,    4,    6,     */{ 6,  1,  2,  6,  5,  1,  4,  7,  8, -1, -1, -1, -1, -1, -1, -1 },
	/*  85: 0,    2,    4,    6,     */{ 1,  2,  5,  5,  2,  6,  3,  0,  4,  3,  4,  7, -1, -1, -1, -1 },
	/*  86:    1, 2,    4,    6,     */{ 8,  4,  7,  9,  0,  5,  0,  6,  5,  0,  2,  6, -1, -1, -1, -1 },
	/*  87: 0, 1, 2,    4,    6,     */{ 7,  3,  9,  7,  9,  4,  3,  2,  9,  5,  9,  6,  2,  6,  9, -1 },
	/*  88:          3, 4,    6,     */{ 3, 11,  2,  7,  8,  4, 10,  6,  5, -1, -1, -1, -1, -1, -1, -1 },
	/*  89: 0,       3, 4,    6,     */{ 5, 10,  6,  4,  7,  2,  4,  2,  0,  2,  7, 11, -1, -1, -1, -1 },
	/*  90:    1,    3, 4,    6,     */{ 0,  1,  9,  4,  7,  8,  2,  3, 11,  5, 10,  6, -1, -1, -1, -1 },
	/*  91: 0, 1,    3, 4,    6,     */{ 9,  2,  1,  9, 11,  2,  9,  4, 11,  7, 11,  4,  5, 10,  6, -1 },
	/*  92:       2, 3, 4,    6,     */{ 8,  4,  7,  3, 11,  5,  3,  5,  1,  5, 11,  6, -1, -1, -1, -1 },
	/*  93: 0,    2, 3, 4,    6,     */{ 5,  1, 11,  5, 11,  6,  1,  0, 11,  7, 11,  4,  0,  4, 11, -1 },
	/*  94:    1, 2, 3, 4,    6,     */{ 0,  5,  9,  0,  6,  5,  0,  3,  6, 11,  6,  3,  8,  4,  7, -1 },
	/*  95: 0, 1, 2, 3, 4,    6,     */{ 6,  5,  9,  6,  9, 11,  4,  7,  9,  7, 11,  9, -1, -1, -1, -1 },
	/*  96:                5, 6,     */{ 10,  4,  9,  6,  4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/*  97: 0,             5, 6,     */{ 4, 10,  6,  4,  9, 10,  0,  8,  3, -1, -1, -1, -1, -1, -1, -1 },
	/*  98:    1,          5, 6,     */{ 10,  0,  1, 10,  6,  0,  6,  4,  0, -1, -1, -1, -1, -1, -1, -1 },
	/*  99: 0, 1,          5, 6,     */{ 8,  3,  1,  8,  1,  6,  8,  6,  4,  6,  1, 10, -1, -1, -1, -1 },
	/* 100:       2,       5, 6,     */{ 1,  4,  9,  1,  2,  4,  2,  6,  4, -1, -1, -1, -1, -1, -1, -1 },
	/* 101: 0,    2,       5, 6,     */{ 3,  0,  8,  1,  2,  9,  2,  4,  9,  2,  6,  4, -1, -1, -1, -1 },
	/* 102:    1, 2,       5, 6,     */{ 0,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 103: 0, 1, 2,       5, 6,     */{ 8,  3,  2,  8,  2,  4,  4,  2,  6, -1, -1, -1, -1, -1, -1, -1 },
	/* 104:          3,    5, 6,     */{ 10,  4,  9, 10,  6,  4, 11,  2,  3, -1, -1, -1, -1, -1, -1, -1 },
	/* 105: 0,       3,    5, 6,     */{ 0,  8,  2,  2,  8, 11,  4,  9, 10,  4, 10,  6, -1, -1, -1, -1 },
	/* 106:    1,    3,    5, 6,     */{ 3, 11,  2,  0,  1,  6,  0,  6,  4,  6,  1, 10, -1, -1, -1, -1 },
	/* 107: 0, 1,    3,    5, 6,     */{ 6,  4,  1,  6,  1, 10,  4,  8,  1,  2,  1, 11,  8, 11,  1, -1 },
	/* 108:       2, 3,    5, 6,     */{ 9,  6,  4,  9,  3,  6,  9,  1,  3, 11,  6,  3, -1, -1, -1, -1 },
	/* 109: 0,    2, 3,    5, 6,     */{ 8, 11,  1,  8,  1,  0, 11,  6,  1,  9,  1,  4,  6,  4,  1, -1 },
	/* 110:    1, 2, 3,    5, 6,     */{ 3, 11,  6,  3,  6,  0,  0,  6,  4, -1, -1, -1, -1, -1, -1, -1 },
	/* 111: 0, 1, 2, 3,    5, 6,     */{ 6,  4,  8, 11,  6,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 112:             4, 5, 6,     */{ 7, 10,  6,  7,  8, 10,  8,  9, 10, -1, -1, -1, -1, -1, -1, -1 },
	/* 113: 0,          4, 5, 6,     */{ 0,  7,  3,  0, 10,  7,  0,  9, 10,  6,  7, 10, -1, -1, -1, -1 },
	/* 114:    1,       4, 5, 6,     */{ 10,  6,  7,  1, 10,  7,  1,  7,  8,  1,  8,  0, -1, -1, -1, -1 },
	/* 115: 0, 1,       4, 5, 6,     */{ 10,  6,  7, 10,  7,  1,  1,  7,  3, -1, -1, -1, -1, -1, -1, -1 },
	/* 116:       2,    4, 5, 6,     */{ 1,  2,  6,  1,  6,  8,  1,  8,  9,  8,  6,  7, -1, -1, -1, -1 },
	/* 117: 0,    2,    4, 5, 6,     */{ 2,  6,  9,  2,  9,  1,  6,  7,  9,  0,  9,  3,  7,  3,  9, -1 },
	/* 118:    1, 2,    4, 5, 6,     */{ 7,  8,  0,  7,  0,  6,  6,  0,  2, -1, -1, -1, -1, -1, -1, -1 },
	/* 119: 0, 1, 2,    4, 5, 6,     */{ 7,  3,  2,  6,  7,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 120:          3, 4, 5, 6,     */{ 2,  3, 11, 10,  6,  8, 10,  8,  9,  8,  6,  7, -1, -1, -1, -1 },
	/* 121: 0,       3, 4, 5, 6,     */{ 2,  0,  7,  2,  7, 11,  0,  9,  7,  6,  7, 10,  9, 10,  7, -1 },
	/* 122:    1,    3, 4, 5, 6,     */{ 1,  8,  0,  1,  7,  8,  1, 10,  7,  6,  7, 10,  2,  3, 11, -1 },
	/* 123: 0, 1,    3, 4, 5, 6,     */{ 11,  2,  1, 11,  1,  7, 10,  6,  1,  6,  7,  1, -1, -1, -1, -1 },
	/* 124:       2, 3, 4, 5, 6,     */{ 8,  9,  6,  8,  6,  7,  9,  1,  6, 11,  6,  3,  1,  3,  6, -1 },
	/* 125: 0,    2, 3, 4, 5, 6,     */{ 0,  9,  1, 11,  6,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 126:    1, 2, 3, 4, 5, 6,     */{ 7,  8,  0,  7,  0,  6,  3, 11,  0, 11,  6,  0, -1, -1, -1, -1 },
	/* 127: 0, 1, 2, 3, 4, 5, 6,     */{ 7, 11,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 128:                      7,  */{ 7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 129: 0,                   7,  */{ 3,  0,  8, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 130:    1,                7,  */{ 0,  1,  9, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 131: 0, 1,                7,  */{ 8,  1,  9,  8,  3,  1, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1 },
	/* 132:       2,             7,  */{ 10,  1,  2,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 133: 0,    2,             7,  */{ 1,  2, 10,  3,  0,  8,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1 },
	/* 134:    1, 2,             7,  */{ 2,  9,  0,  2, 10,  9,  6, 11,  7, -1, -1, -1, -1, -1, -1, -1 },
	/* 135: 0, 1, 2,             7,  */{ 6, 11,  7,  2, 10,  3, 10,  8,  3, 10,  9,  8, -1, -1, -1, -1 },
	/* 136:          3,          7,  */{ 7,  2,  3,  6,  2,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 137: 0,       3,          7,  */{ 7,  0,  8,  7,  6,  0,  6,  2,  0, -1, -1, -1, -1, -1, -1, -1 },
	/* 138:    1,    3,          7,  */{ 2,  7,  6,  2,  3,  7,  0,  1,  9, -1, -1, -1, -1, -1, -1, -1 },
	/* 139: 0, 1,    3,          7,  */{ 1,  6,  2,  1,  8,  6,  1,  9,  8,  8,  7,  6, -1, -1, -1, -1 },
	/* 140:       2, 3,          7,  */{ 10,  7,  6, 10,  1,  7,  1,  3,  7, -1, -1, -1, -1, -1, -1, -1 },
	/* 141: 0,    2, 3,          7,  */{ 10,  7,  6,  1,  7, 10,  1,  8,  7,  1,  0,  8, -1, -1, -1, -1 },
	/* 142:    1, 2, 3,          7,  */{ 0,  3,  7,  0,  7, 10,  0, 10,  9,  6, 10,  7, -1, -1, -1, -1 },
	/* 143: 0, 1, 2, 3,          7,  */{ 7,  6, 10,  7, 10,  8,  8, 10,  9, -1, -1, -1, -1, -1, -1, -1 },
	/* 144:             4,       7,  */{ 6,  8,  4, 11,  8,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 145: 0,          4,       7,  */{ 3,  6, 11,  3,  0,  6,  0,  4,  6, -1, -1, -1, -1, -1, -1, -1 },
	/* 146:    1,       4,       7,  */{ 8,  6, 11,  8,  4,  6,  9,  0,  1, -1, -1, -1, -1, -1, -1, -1 },
	/* 147: 0, 1,       4,       7,  */{ 9,  4,  6,  9,  6,  3,  9,  3,  1, 11,  3,  6, -1, -1, -1, -1 },
	/* 148:       2,    4,       7,  */{ 6,  8,  4,  6, 11,  8,  2, 10,  1, -1, -1, -1, -1, -1, -1, -1 },
	/* 149: 0,    2,    4,       7,  */{ 1,  2, 10,  3,  0, 11,  0,  6, 11,  0,  4,  6, -1, -1, -1, -1 },
	/* 150:    1, 2,    4,       7,  */{ 4, 11,  8,  4,  6, 11,  0,  2,  9,  2, 10,  9, -1, -1, -1, -1 },
	/* 151: 0, 1, 2,    4,       7,  */{ 10,  9,  3, 10,  3,  2,  9,  4,  3, 11,  3,  6,  4,  6,  3, -1 },
	/* 152:          3, 4,       7,  */{ 8,  2,  3,  8,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1 },
	/* 153: 0,       3, 4,       7,  */{ 0,  4,  2,  4,  6,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 154:    1,    3, 4,       7,  */{ 1,  9,  0,  2,  3,  4,  2,  4,  6,  4,  3,  8, -1, -1, -1, -1 },
	/* 155: 0, 1,    3, 4,       7,  */{ 1,  9,  4,  1,  4,  2,  2,  4,  6, -1, -1, -1, -1, -1, -1, -1 },
	/* 156:       2, 3, 4,       7,  */{ 8,  1,  3,  8,  6,  1,  8,  4,  6,  6, 10,  1, -1, -1, -1, -1 },
	/* 157: 0,    2, 3, 4,       7,  */{ 10,  1,  0, 10,  0,  6,  6,  0,  4, -1, -1, -1, -1, -1, -1, -1 },
	/* 158:    1, 2, 3, 4,       7,  */{ 4,  6,  3,  4,  3,  8,  6, 10,  3,  0,  3,  9, 10,  9,  3, -1 },
	/* 159: 0, 1, 2, 3, 4,       7,  */{ 10,  9,  4,  6, 10,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 160:                5,    7,  */{ 4,  9,  5,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 161: 0,             5,    7,  */{ 0,  8,  3,  4,  9,  5, 11,  7,  6, -1, -1, -1, -1, -1, -1, -1 },
	/* 162:    1,          5,    7,  */{ 5,  0,  1,  5,  4,  0,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1 },
	/* 163: 0, 1,          5,    7,  */{ 11,  7,  6,  8,  3,  4,  3,  5,  4,  3,  1,  5, -1, -1, -1, -1 },
	/* 164:       2,       5,    7,  */{ 9,  5,  4, 10,  1,  2,  7,  6, 11, -1, -1, -1, -1, -1, -1, -1 },
	/* 165: 0,    2,       5,    7,  */{ 6, 11,  7,  1,  2, 10,  0,  8,  3,  4,  9,  5, -1, -1, -1, -1 },
	/* 166:    1, 2,       5,    7,  */{ 7,  6, 11,  5,  4, 10,  4,  2, 10,  4,  0,  2, -1, -1, -1, -1 },
	/* 167: 0, 1, 2,       5,    7,  */{ 3,  4,  8,  3,  5,  4,  3,  2,  5, 10,  5,  2, 11,  7,  6, -1 },
	/* 168:          3,    5,    7,  */{ 7,  2,  3,  7,  6,  2,  5,  4,  9, -1, -1, -1, -1, -1, -1, -1 },
	/* 169: 0,       3,    5,    7,  */{ 9,  5,  4,  0,  8,  6,  0,  6,  2,  6,  8,  7, -1, -1, -1, -1 },
	/* 170:    1,    3,    5,    7,  */{ 3,  6,  2,  3,  7,  6,  1,  5,  0,  5,  4,  0, -1, -1, -1, -1 },
	/* 171: 0, 1,    3,    5,    7,  */{ 6,  2,  8,  6,  8,  7,  2,  1,  8,  4,  8,  5,  1,  5,  8, -1 },
	/* 172:       2, 3,    5,    7,  */{ 9,  5,  4, 10,  1,  6,  1,  7,  6,  1,  3,  7, -1, -1, -1, -1 },
	/* 173: 0,    2, 3,    5,    7,  */{ 1,  6, 10,  1,  7,  6,  1,  0,  7,  8,  7,  0,  9,  5,  4, -1 },
	/* 174:    1, 2, 3,    5,    7,  */{ 4,  0, 10,  4, 10,  5,  0,  3, 10,  6, 10,  7,  3,  7, 10, -1 },
	/* 175: 0, 1, 2, 3,    5,    7,  */{ 7,  6, 10,  7, 10,  8,  5,  4, 10,  4,  8, 10, -1, -1, -1, -1 },
	/* 176:             4, 5,    7,  */{ 6,  9,  5,  6, 11,  9, 11,  8,  9, -1, -1, -1, -1, -1, -1, -1 },
	/* 177: 0,          4, 5,    7,  */{ 3,  6, 11,  0,  6,  3,  0,  5,  6,  0,  9,  5, -1, -1, -1, -1 },
	/* 178:    1,       4, 5,    7,  */{ 0, 11,  8,  0,  5, 11,  0,  1,  5,  5,  6, 11, -1, -1, -1, -1 },
	/* 179: 0, 1,       4, 5,    7,  */{ 6, 11,  3,  6,  3,  5,  5,  3,  1, -1, -1, -1, -1, -1, -1, -1 },
	/* 180:       2,    4, 5,    7,  */{ 1,  2, 10,  9,  5, 11,  9, 11,  8, 11,  5,  6, -1, -1, -1, -1 },
	/* 181: 0,    2,    4, 5,    7,  */{ 0, 11,  3,  0,  6, 11,  0,  9,  6,  5,  6,  9,  1,  2, 10, -1 },
	/* 182:    1, 2,    4, 5,    7,  */{ 11,  8,  5, 11,  5,  6,  8,  0,  5, 10,  5,  2,  0,  2,  5, -1 },
	/* 183: 0, 1, 2,    4, 5,    7,  */{ 6, 11,  3,  6,  3,  5,  2, 10,  3, 10,  5,  3, -1, -1, -1, -1 },
	/* 184:          3, 4, 5,    7,  */{ 5,  8,  9,  5,  2,  8,  5,  6,  2,  3,  8,  2, -1, -1, -1, -1 },
	/* 185: 0,       3, 4, 5,    7,  */{ 9,  5,  6,  9,  6,  0,  0,  6,  2, -1, -1, -1, -1, -1, -1, -1 },
	/* 186:    1,    3, 4, 5,    7,  */{ 1,  5,  8,  1,  8,  0,  5,  6,  8,  3,  8,  2,  6,  2,  8, -1 },
	/* 187: 0, 1,    3, 4, 5,    7,  */{ 1,  5,  6,  2,  1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 188:       2, 3, 4, 5,    7,  */{ 1,  3,  6,  1,  6, 10,  3,  8,  6,  5,  6,  9,  8,  9,  6, -1 },
	/* 189: 0,    2, 3, 4, 5,    7,  */{ 10,  1,  0, 10,  0,  6,  9,  5,  0,  5,  6,  0, -1, -1, -1, -1 },
	/* 190:    1, 2, 3, 4, 5,    7,  */{ 0,  3,  8,  5,  6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 191: 0, 1, 2, 3, 4, 5,    7,  */{ 10,  5,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 192:                   6, 7,  */{ 11,  5, 10,  7,  5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 193: 0,                6, 7,  */{ 11,  5, 10, 11,  7,  5,  8,  3,  0, -1, -1, -1, -1, -1, -1, -1 },
	/* 194:    1,             6, 7,  */{ 5, 11,  7,  5, 10, 11,  1,  9,  0, -1, -1, -1, -1, -1, -1, -1 },
	/* 195: 0, 1,             6, 7,  */{ 10,  7,  5, 10, 11,  7,  9,  8,  1,  8,  3,  1, -1, -1, -1, -1 },
	/* 196:       2,          6, 7,  */{ 11,  1,  2, 11,  7,  1,  7,  5,  1, -1, -1, -1, -1, -1, -1, -1 },
	/* 197: 0,    2,          6, 7,  */{ 0,  8,  3,  1,  2,  7,  1,  7,  5,  7,  2, 11, -1, -1, -1, -1 },
	/* 198:    1, 2,          6, 7,  */{ 9,  7,  5,  9,  2,  7,  9,  0,  2,  2, 11,  7, -1, -1, -1, -1 },
	/* 199: 0, 1, 2,          6, 7,  */{ 7,  5,  2,  7,  2, 11,  5,  9,  2,  3,  2,  8,  9,  8,  2, -1 },
	/* 200:          3,       6, 7,  */{ 2,  5, 10,  2,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1 },
	/* 201: 0,       3,       6, 7,  */{ 8,  2,  0,  8,  5,  2,  8,  7,  5, 10,  2,  5, -1, -1, -1, -1 },
	/* 202:    1,    3,       6, 7,  */{ 9,  0,  1,  5, 10,  3,  5,  3,  7,  3, 10,  2, -1, -1, -1, -1 },
	/* 203: 0, 1,    3,       6, 7,  */{ 9,  8,  2,  9,  2,  1,  8,  7,  2, 10,  2,  5,  7,  5,  2, -1 },
	/* 204:       2, 3,       6, 7,  */{ 1,  3,  5,  3,  7,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 205: 0,    2, 3,       6, 7,  */{ 0,  8,  7,  0,  7,  1,  1,  7,  5, -1, -1, -1, -1, -1, -1, -1 },
	/* 206:    1, 2, 3,       6, 7,  */{ 9,  0,  3,  9,  3,  5,  5,  3,  7, -1, -1, -1, -1, -1, -1, -1 },
	/* 207: 0, 1, 2, 3,       6, 7,  */{ 9,  8,  7,  5,  9,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 208:             4,    6, 7,  */{ 5,  8,  4,  5, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1 },
	/* 209: 0,          4,    6, 7,  */{ 5,  0,  4,  5, 11,  0,  5, 10, 11, 11,  3,  0, -1, -1, -1, -1 },
	/* 210:    1,       4,    6, 7,  */{ 0,  1,  9,  8,  4, 10,  8, 10, 11, 10,  4,  5, -1, -1, -1, -1 },
	/* 211: 0, 1,       4,    6, 7,  */{ 10, 11,  4, 10,  4,  5, 11,  3,  4,  9,  4,  1,  3,  1,  4, -1 },
	/* 212:       2,    4,    6, 7,  */{ 2,  5,  1,  2,  8,  5,  2, 11,  8,  4,  5,  8, -1, -1, -1, -1 },
	/* 213: 0,    2,    4,    6, 7,  */{ 0,  4, 11,  0, 11,  3,  4,  5, 11,  2, 11,  1,  5,  1, 11, -1 },
	/* 214:    1, 2,    4,    6, 7,  */{ 0,  2,  5,  0,  5,  9,  2, 11,  5,  4,  5,  8, 11,  8,  5, -1 },
	/* 215: 0, 1, 2,    4,    6, 7,  */{ 9,  4,  5,  2, 11,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 216:          3, 4,    6, 7,  */{ 2,  5, 10,  3,  5,  2,  3,  4,  5,  3,  8,  4, -1, -1, -1, -1 },
	/* 217: 0,       3, 4,    6, 7,  */{ 5, 10,  2,  5,  2,  4,  4,  2,  0, -1, -1, -1, -1, -1, -1, -1 },
	/* 218:    1,    3, 4,    6, 7,  */{ 3, 10,  2,  3,  5, 10,  3,  8,  5,  4,  5,  8,  0,  1,  9, -1 },
	/* 219: 0, 1,    3, 4,    6, 7,  */{ 5, 10,  2,  5,  2,  4,  1,  9,  2,  9,  4,  2, -1, -1, -1, -1 },
	/* 220:       2, 3, 4,    6, 7,  */{ 8,  4,  5,  8,  5,  3,  3,  5,  1, -1, -1, -1, -1, -1, -1, -1 },
	/* 221: 0,    2, 3, 4,    6, 7,  */{ 0,  4,  5,  1,  0,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 222:    1, 2, 3, 4,    6, 7,  */{ 8,  4,  5,  8,  5,  3,  9,  0,  5,  0,  3,  5, -1, -1, -1, -1 },
	/* 223: 0, 1, 2, 3, 4,    6, 7,  */{ 9,  4,  5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 224:                5, 6, 7,  */{ 4, 11,  7,  4,  9, 11,  9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
	/* 225: 0,             5, 6, 7,  */{ 0,  8,  3,  4,  9,  7,  9, 11,  7,  9, 10, 11, -1, -1, -1, -1 },
	/* 226:    1,          5, 6, 7,  */{ 1, 10, 11,  1, 11,  4,  1,  4,  0,  7,  4, 11, -1, -1, -1, -1 },
	/* 227: 0, 1,          5, 6, 7,  */{ 3,  1,  4,  3,  4,  8,  1, 10,  4,  7,  4, 11, 10, 11,  4, -1 },
	/* 228:       2,       5, 6, 7,  */{ 4, 11,  7,  9, 11,  4,  9,  2, 11,  9,  1,  2, -1, -1, -1, -1 },
	/* 229: 0,    2,       5, 6, 7,  */{ 9,  7,  4,  9, 11,  7,  9,  1, 11,  2, 11,  1,  0,  8,  3, -1 },
	/* 230:    1, 2,       5, 6, 7,  */{ 11,  7,  4, 11,  4,  2,  2,  4,  0, -1, -1, -1, -1, -1, -1, -1 },
	/* 231: 0, 1, 2,       5, 6, 7,  */{ 11,  7,  4, 11,  4,  2,  8,  3,  4,  3,  2,  4, -1, -1, -1, -1 },
	/* 232:          3,    5, 6, 7,  */{ 2,  9, 10,  2,  7,  9,  2,  3,  7,  7,  4,  9, -1, -1, -1, -1 },
	/* 233: 0,       3,    5, 6, 7,  */{ 9, 10,  7,  9,  7,  4, 10,  2,  7,  8,  7,  0,  2,  0,  7, -1 },
	/* 234:    1,    3,    5, 6, 7,  */{ 3,  7, 10,  3, 10,  2,  7,  4, 10,  1, 10,  0,  4,  0, 10, -1 },
	/* 235: 0, 1,    3,    5, 6, 7,  */{ 1, 10,  2,  8,  7,  4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 236:       2, 3,    5, 6, 7,  */{ 4,  9,  1,  4,  1,  7,  7,  1,  3, -1, -1, -1, -1, -1, -1, -1 },
	/* 237: 0,    2, 3,    5, 6, 7,  */{ 4,  9,  1,  4,  1,  7,  0,  8,  1,  8,  7,  1, -1, -1, -1, -1 },
	/* 238:    1, 2, 3,    5, 6, 7,  */{ 4,  0,  3,  7,  4,  3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 239: 0, 1, 2, 3,    5, 6, 7,  */{ 4,  8,  7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 240:             4, 5, 6, 7,  */{ 9, 10,  8, 10, 11,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 241: 0,          4, 5, 6, 7,  */{ 3,  0,  9,  3,  9, 11, 11,  9, 10, -1, -1, -1, -1, -1, -1, -1 },
	/* 242:    1,       4, 5, 6, 7,  */{ 0,  1, 10,  0, 10,  8,  8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
	/* 243: 0, 1,       4, 5, 6, 7,  */{ 3,  1, 10, 11,  3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 244:       2,    4, 5, 6, 7,  */{ 1,  2, 11,  1, 11,  9,  9, 11,  8, -1, -1, -1, -1, -1, -1, -1 },
	/* 245: 0,    2,    4, 5, 6, 7,  */{ 3,  0,  9,  3,  9, 11,  1,  2,  9,  2, 11,  9, -1, -1, -1, -1 },
	/* 246:    1, 2,    4, 5, 6, 7,  */{ 0,  2, 11,  8,  0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 247: 0, 1, 2,    4, 5, 6, 7,  */{ 3,  2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 248:          3, 4, 5, 6, 7,  */{ 2,  3,  8,  2,  8, 10, 10,  8,  9, -1, -1, -1, -1, -1, -1, -1 },
	/* 249: 0,       3, 4, 5, 6, 7,  */{ 9, 10,  2,  0,  9,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 250:    1,    3, 4, 5, 6, 7,  */{ 2,  3,  8,  2,  8, 10,  0,  1,  8,  1, 10,  8, -1, -1, -1, -1 },
	/* 251: 0, 1,    3, 4, 5, 6, 7,  */{ 1, 10,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 252:       2, 3, 4, 5, 6, 7,  */{ 1,  3,  8,  9,  1,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 253: 0,    2, 3, 4, 5, 6, 7,  */{ 0,  9,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 254:    1, 2, 3, 4, 5, 6, 7,  */{ 0,  3,  8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
	/* 255: 0, 1, 2, 3, 4, 5, 6, 7,  */{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
};