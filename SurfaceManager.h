#pragma once
#include <GL/glew.h>  
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Eigen\Core>
#include <vector>
#include "SPHKernel.h"
#include "ShaderManager.h"
#include "CompactNSearch.h"
#include <memory>
#include "SPHManager.h"

class SurfaceManager {

public:
	SurfaceManager() 
	{}
	~SurfaceManager() 
	{}

	void Init(SPHManager* pSPHManager);

	struct Mesh {
		Mesh() {}
		~Mesh(){}
		size_t i_max, j_max, k_max;
		std::vector<Eigen::Vector3d> Pos;

	};
	
	void Draw();
	ShaderManager m_ShaderManager;
	void CreateSurface();

	bool SaveVertexPosToFile(std::ofstream& file);
	bool LoadVertexPosFromFile(std::ifstream& file);

private:

private:
	ParticleManager* m_pParticleManager;

	std::vector<GLfloat> m_SurfaceVertices;
	std::vector<float> m_VoxelWeights;

	GLfloat m_CubeWidth = 0.06f;
	GLuint m_iVertexArrayObject;

	const float m_kSurfaceConst = 0.0f;

	Eigen::Vector3d m_LowerLeftBackVec;

	Mesh m_Mesh;
	SPH::Kernel* m_pKernel;
	std::unique_ptr<CompactNSearch> m_pCompactNSearch;
	unsigned int m_fluidDiscretizationId;
	unsigned int m_surfaceDiscretizationId;
	std::vector<double>* m_density;

	static int8_t const marching_cubes_lut[256][16];

	std::ofstream out;
	std::ifstream in;

};