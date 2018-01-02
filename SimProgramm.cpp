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

	//std::random_device oRandomDevice;
	//std::mt19937 Gen(oRandomDevice());
	//std::uniform_real_distribution<double> Dis(-30.0, 30.0);
	//for (int i = 0; i < 1000; i++)
	//{
	//	SimSystem::GetInstance()->GetParticleManager()->AddParticle(Eigen::Vector3d(Dis(Gen), Dis(Gen), Dis(Gen)));
	//}

    static constexpr int boxWidth = 20;
	ParticleManager *x = SimSystem::GetInstance()->GetSPHManager()->GetParticleManager();
	x->GetParticlePositions()->reserve(6 * x->m_iBoundariesPerFaceInOneDirection * x->m_iBoundariesPerFaceInOneDirection + boxWidth * boxWidth * boxWidth);

    //SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->AddParticle(Eigen::Vector3d(0.0f, 0.5f, 0.0f));
    //SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->AddParticle(Eigen::Vector3d(0.0f, 0.25f, 0.0f));


	for (int i = 0; i < boxWidth; i++)
	{
		for (int j = 0; j < boxWidth; j++)
		{	
			for (int k = 0; k < boxWidth; k++)
			{
                auto pos = Eigen::Vector3d((i - (boxWidth / 2)) * 0.1, (j - (boxWidth / 2)) * 0.1, (k - (boxWidth / 2)) * 0.1);
                pos += Eigen::Vector3d(1.0, 0.0, 0.0);
				SimSystem::GetInstance()->GetSPHManager()->GetParticleManager()->AddParticle(pos);
			}
		}
	}


}