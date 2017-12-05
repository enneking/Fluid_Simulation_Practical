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
	SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->GetParticlePositions()->reserve(1000);

	//std::random_device oRandomDevice;
	//std::mt19937 Gen(oRandomDevice());
	//std::uniform_real_distribution<double> Dis(-30.0, 30.0);
	//for (int i = 0; i < 1000; i++)
	//{
	//	SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(Dis(Gen), Dis(Gen), Dis(Gen)));
	//}

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 10; j++)
		{	
			for (int k = 0; k < 10; k++)
			{
				SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->AddParticle(Eigen::Vector3d( (i-5) * 0.2f, (j-5) * 0.2f, (k-5) * 0.2f));
			}
		}
	}


}