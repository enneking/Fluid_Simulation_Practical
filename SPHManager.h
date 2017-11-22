#pragma once
#include <Eigen\Core>
#include "ParticleManager.h"

class SPHManager
{
public:
	SPHManager();
	~SPHManager();

	void Init();

	void Update(double dt);

	ParticleManager* GetParticleManager();

private:
	void CalculateGravitation();

private:
	double m_fGravityForce = -0.00981;
	ParticleManager m_oParticleManager;
};

