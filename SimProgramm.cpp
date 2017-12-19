#include "SimProgramm.h"


SimProgramm::SimProgramm()
{
}


SimProgramm::~SimProgramm()
{
}

void SimProgramm::Run(int argc, char* argv[])
{
	SimSystem::GetInstance()->Init(argc, argv);

	InitScene();

	SimSystem::GetInstance()->Run();
}


void SimProgramm::InitScene()
{
	SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->GetParticlePositions()->reserve(100000);

    static constexpr int boxWidth = 20;

	for (int i = 0; i < boxWidth; i++)
	{
		for (int j = 0; j < boxWidth; j++)
		{	
			for (int k = 0; k < boxWidth; k++)
			{
				SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->AddParticle(Eigen::Vector3d( (i-(boxWidth / 2)) * 0.1f, (j-(boxWidth / 2)) * 0.1f, (k-(boxWidth / 2)) * 0.1f));
			}
		}
	}


}