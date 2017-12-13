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
	int GetBoundarieParticleCount();

	int m_iBoundariesPerFaceInOneDirection = 25;
	Eigen::Vector3d m_vBoxLeftLowerBack = Eigen::Vector3d(-1.1f, -1.1f, -1.1f);
	Eigen::Vector3d m_vBoxRightUpperFront = Eigen::Vector3d(1.1f, 1.1f, 1.1f);

    bool SerialiseStateToFile(std::ofstream& file);
    bool LoadStateFromFile(std::ifstream& file);

private:
	double m_fParticleMass = 20.0;
	std::vector<Particle> m_vParticleContainer;
	std::vector<Eigen::Vector3d> m_vParticlePositions;

	GLushort m_vBoundaryIndices[36];

	int m_iBoundaryCount = m_iBoundariesPerFaceInOneDirection * m_iBoundariesPerFaceInOneDirection * 6;	

	GLuint m_iVertexBufferObject, m_iVertexArrayObject, m_iProgramID, m_iVaoLine, m_iVaoBox, Vao;
	ShaderManager m_oShaderManager;

	Camera* m_pCamera;

};

