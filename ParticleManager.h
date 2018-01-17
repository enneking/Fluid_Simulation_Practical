#pragma once
#include <GL/glew.h>  
#include <GLFW/glfw3.h>

#include "Field.h"
#include <fstream>

#include <Eigen\Core>

#include "ShaderManager.h"
#include "Camera.h"



class SimSystem;

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	struct Particle {
		Particle();
		~Particle();

		Eigen::Vector3d m_vVelocity = Eigen::Vector3d(0.0, 0.0, 0.0);
		size_t m_pPositionIdx;  // use indices instead of pointers because reallocation will not invalidate them 
	};

	void Init(Camera* pCamera);

	void InitBuffers();

	void SetUpBoundaryBox();

	Particle* AddParticle(Eigen::Vector3d fInitialPos = Eigen::Vector3d::Zero(), Eigen::Vector3d fInitialVelocity = Eigen::Vector3d::Zero());

	void DrawParticles();

	double GetParticleMass();
	void SetParticleMass(double value);

	std::vector<Particle>* GetParticleContainer();
	std::vector<Eigen::Vector3d>* GetParticlePositions();
	Eigen::Vector3d* GetBoundaryPositions();
	int GetBoundaryParticleCount();


	int m_iBoundariesPerFaceInOneDirection = 200;
	Eigen::Vector3d m_vBoxLeftLowerBack = Eigen::Vector3d(-2.5f, -1.8, -1.7f);
	Eigen::Vector3d m_vBoxRightUpperFront = Eigen::Vector3d(2.7f, 2.1f, 1.6f);

    bool SerialiseStateToFile(std::ofstream& file);
    bool LoadStateFromFile(std::ifstream& file);

	Camera* GetCamera() { return m_pCamera; }

private:
	double m_fParticleMass = 1.0;
	float m_fParticleRadius = 0.05f;
	std::vector<Particle> m_vParticleContainer;
	std::vector<Eigen::Vector3d> m_vParticlePositions;

	GLushort m_vBoundaryIndices[36];

	int m_iBoundaryCount = m_iBoundariesPerFaceInOneDirection * m_iBoundariesPerFaceInOneDirection * 6;	

	GLuint m_iVertexBufferObject, m_iVertexArrayObject, m_iProgramID, m_iVaoLine, m_iVaoBox, Vao;
	ShaderManager m_oShaderManager;

	Camera* m_pCamera;

};

