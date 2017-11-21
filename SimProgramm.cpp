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
	SimSystem::GetInstance()->GetParticleManager()->GetParticlePositions()->reserve(1000);

	//std::random_device oRandomDevice;
	//std::mt19937 Gen(oRandomDevice());
	//std::uniform_real_distribution<double> Dis(-30.0, 30.0);
	//for (int i = 0; i < 1000; i++)
	//{
	//	SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(Dis(Gen), Dis(Gen), Dis(Gen)));
	//}
	SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(0.0, 0.0, 0.0f));
	SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(0.5, 0.0, 0.0f));
	SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(0.0, 0.5, 0.0f));

}