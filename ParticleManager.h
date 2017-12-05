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
		Eigen::Vector3d* m_pPosition;
	};

	void Init(Camera* pCamera);

	void InitBuffers();

	void SetUpBoundaryBox();

	Particle* AddParticle(Eigen::Vector3d fInitialPos = Eigen::Vector3d::Zero(), Eigen::Vector3d fInitialVelocity = Eigen::Vector3d::Zero());

	void DrawParticles();

	float GetParticleMass();
	void SetParticleMass(float value);

	void MoveParticles(double dt);

	std::vector<Particle>* GetParticleContainer();
	std::vector<Eigen::Vector3d>* GetParticlePositions();
	Eigen::Vector3d* GetBoundaryPositions();

	int m_iBoundariesPerFaceInOneDirection = 10;
	Eigen::Vector3d m_vBoxLeftLowerBack = Eigen::Vector3d(-1.1f, -1.1f, -1.1f);
	Eigen::Vector3d m_vBoxRightUpperFront = Eigen::Vector3d(1.1f, 1.1f, 1.1f);



private:
	double m_fParticleMass = 20.0;
	std::vector<Particle> m_vParticleContainer;
	std::vector<Eigen::Vector3d> m_vParticlePositions;

	Eigen::Vector3d m_vBoundaryPositions[8];
	GLushort m_vBoundaryIndices[36];

	GLuint m_iVertexBufferObject, m_iVertexArrayObject, m_iProgramID, m_iVaoLine, m_iVaoBox;
	ShaderManager m_oShaderManager;

	Camera* m_pCamera;

};

