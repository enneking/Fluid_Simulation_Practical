#pragma once
#include "SimViz.h"
#include "ParticleManager.h"
#include <memory>



class SimSystem
{
public:
	static SimSystem* GetInstance();

	void Init(int argc, char* argv[]);
	void Run();

	void closeFunc();

	ParticleManager* GetParticleManager();
	SimViz* GetSimViz();


private:	
	SimSystem::SimSystem();
	SimSystem::~SimSystem();

	std::unique_ptr<SimViz> m_pSimViz;
	ParticleManager m_oParticleManager;
	static SimSystem s_oSimSystem;
};

