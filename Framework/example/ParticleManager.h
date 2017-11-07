#pragma once
#include <GLviz>
#include "Field.h"
struct Particle {
	Particle();
	~Particle();

	Eigen::Vector3d m_vVelocity;
	double* m_pPosition;
};

class ParticleManager
{
public:
	ParticleManager();
	~ParticleManager();

	void InitBuffers();

	void AddParticle(Eigen::Vector3d fInitialPos = Eigen::Vector3d::Zero(), Eigen::Vector3d fInitialVelocity = Eigen::Vector3d::Zero());

	//void DrawParticles();

	std::vector<Particle>* GetParticleContainer();
	std::vector<Eigen::Vector3d>* GetParticlePositions();

private:
	std::vector<Particle> m_vParticleContainer;
	std::vector<Eigen::Vector3d> m_vParticlePositions;
	GLuint m_iVertexBufferObject;
};

