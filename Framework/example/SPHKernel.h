#pragma once
#include "SimSystem.h"

class SPHKernel
{
public:
	SPHKernel();
	~SPHKernel();

	void Run();

	double CubicSplineKernel(const Eigen::Vector3d& x, double fSmoothingLength);
	double QuinticSplineKernel(const Eigen::Vector3d& x, double h);
	double QuadraticSmoothingFunctionKernel(const Eigen::Vector3d& x, double h);

	Eigen::Vector3d ComputeCentralDifferences( double (SPHKernel::* const KernelFunc)(const Eigen::Vector3d& x, double h), const Eigen::Vector3d& x, double fSmoothingLength);

    Eigen::Vector3d CubicSplineKernelGradient(const Eigen::Vector3d& x, double fSmoothingLength);
    Eigen::Vector3d QuinticSplineKernelGradient(const Eigen::Vector3d& x, double h);
    Eigen::Vector3d QuadraticSmoothingFunctionKernelGradient(const Eigen::Vector3d& x, double h);
private:
};

