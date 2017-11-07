#pragma once
#include "SimSystem.h"

class SPHKernel
{
public:
	SPHKernel();
	~SPHKernel();

	double CubicSplineKernel(const Eigen::Vector3d& x, double fSmoothingLength);
	double QuinticSplineKernel(const Eigen::Vector3d& x, double h);
	double QuadraticSmoothingFunctionKernel(const Eigen::Vector3d& x, double h);

private:
};

