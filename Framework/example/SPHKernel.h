#pragma once
#include "SimSystem.h"

class SPHKernel
{
public:
	SPHKernel();
	~SPHKernel();

	double CubicSplineKernel(float& fDistance, float& fSmoothingLength);

private:
};