#include "SimProgramm.h"

#include "SPHKernel.h"

SimProgramm::SimProgramm()
{
}


SimProgramm::~SimProgramm()
{
}

void SimProgramm::Run(int argc, char* argv[])
{

    SPHKernel kernel;

    const Eigen::Vector3d x = Eigen::Vector3d(0.0, 0.0, 0.0) - Eigen::Vector3d(0.5, 0.5, 0.5);
    const double h = 1.0;

    printf("CubicSplineKernel(x, h) = %f\n", kernel.CubicSplineKernel(x, h));
    printf("QuinticSplineKernel(x, h) = %f\n", kernel.QuinticSplineKernel(x, h));
    printf("QuadraticSmoothingFunctionKernel(x, h) = %f\n", kernel.QuadraticSmoothingFunctionKernel(x, h));

    const double errorCubic =
        Eigen::Vector3d(kernel.CubicSplineKernelGradient(x, h) - kernel.ComputeCentralDifferences(&SPHKernel::CubicSplineKernel, x, h)).norm();
    const double errorQuintic =
        Eigen::Vector3d(kernel.QuinticSplineKernelGradient(x, h) - kernel.ComputeCentralDifferences(&SPHKernel::QuinticSplineKernel, x, h)).norm();
    const double errorQuadraticSmoothingFunction =
        Eigen::Vector3d(kernel.QuadraticSmoothingFunctionKernelGradient(x, h) - kernel.ComputeCentralDifferences(&SPHKernel::QuadraticSmoothingFunctionKernel, x, h)).norm();

    printf("cubic spline kernel gradient absolute error: %f\n", errorCubic);
    printf("quintic spline kernel gradient absolute error: %f\n", errorQuintic);
    printf("quadratic smoothing function kernel gradient absolute error: %f\n", errorQuadraticSmoothingFunction);

    printf("\n--------------------------------------------------------------\n");
    
	SimSystem::GetInstance()->Init(argc, argv);
	
	m_oNeighborhoodSearch.Run();

	SimSystem::GetInstance()->Run();
}
