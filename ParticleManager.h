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

	void AddParticle(Eigen::Vector3d fInitialPos = Eigen::Vector3d::Zero(), Eigen::Vector3d fInitialVelocity = Eigen::Vector3d::Zero());

	void DrawParticles();

	float GetParticleMass();
	void SetParticleMass(float value);

	void MoveParticles(double dt);

	std::vector<Particle>* GetParticleContainer();
	std::vector<Eigen::Vector3d>* GetParticlePositions();

private:
	float m_fParticleMass;
	std::vector<Particle> m_vParticleContainer;
	std::vector<Eigen::Vector3d> m_vParticlePositions;
	GLuint m_iVertexBufferObject, m_iVertexArrayObject, m_iProgramID;
	ShaderManager m_oShaderManager{ "ParticleShader.frag", "ParticleShader.vert"};

	Camera* m_pCamera;

};

