#pragma once
#include "SimSystem.h"

class SPHKernel
{
public:
	SPHKernel();
	~SPHKernel();

	double CubicSplineKernel(float& fDistance, float& fSmoothingLength);
	double QuinticSplineKernel(const Eigen::Vector3d& x, double h);
	double SPHKernel::QuadraticSmoothingFunctionKernel(const Eigen::Vector3d& x, double h);

private:
};

