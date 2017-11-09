#include "SPHKernel.h"

SPHKernel::SPHKernel()
{
}

SPHKernel::~SPHKernel()
{
}

double SPHKernel::CubicSplineKernel(const Eigen::Vector3d& x, double fSmoothingLength)
{
	double dAlpha = 3.0 / (2.0 * M_PI * fSmoothingLength * fSmoothingLength * fSmoothingLength);
	double relDist = x.norm() / fSmoothingLength;

	if (relDist < 1)
	{
		return (2.0 / 3.0) - relDist * relDist + 0.5 * (relDist * relDist * relDist);
	}
	else if(relDist < 2)
	{
		return (1.0 / 6.0) * (2.0 - relDist) * (2.0 - relDist) * (2.0 - relDist);
	}
	else
	{
		return 0.0;
	}
}

double SPHKernel::QuinticSplineKernel(const Eigen::Vector3d& x, double h)
{
	const double R = x.norm() / h;
	const double alpha_d = 3.0 / (359.0 * M_PI * pow(h, 3));

	double mul = 0.0;
	if ((0.0 <= R) && (R < 1.0)) {
		mul = pow((3.0 - R), 5.0) - 6.0 * pow((2.0 - R), 5) + 15.0 * pow((1.0f - R), 5);
	}
	else {
		if (R < 2.0) {
			mul = pow((3.0 - R), 5.0) - 6.0 * pow((2.0 - R), 5);
		}
		else {
			if (R < 3.0) {
				mul = pow((3.0 - R), 5.0);
			}
		}
	}
	return alpha_d * mul;
}

double SPHKernel::QuadraticSmoothingFunctionKernel(const Eigen::Vector3d& x, double h)
{
	const double R = x.norm() / h;
	const double alpha_d = 5.0 / (4.0 * M_PI * pow(h, 3.0));

	assert(R <= 2.0 && "QuadraticSmoothingFunctionKernel: W(x, h) is not defined for R > 2");

	return alpha_d * ((3.0 / 16.0) * pow(R, 2.0) - (3.0 / 4.0) * R + (3.0 / 4.0));
}

Eigen::Vector3d SPHKernel::ComputeCentralDifferences(double (SPHKernel::* const KernelFunc)(const Eigen::Vector3d& x, double h),const Eigen::Vector3d& x, double fSmoothingLength)
{
	const double epsilon = 0.000001;
	Eigen::Vector3d vCentralDif;
	vCentralDif[0] = (this->*(KernelFunc))(x + epsilon * Eigen::Vector3d(1.0, 0.0, 0.0), fSmoothingLength)
					- (this->*(KernelFunc))(x - epsilon * Eigen::Vector3d(1.0, 0.0, 0.0), fSmoothingLength);

	vCentralDif[1] = (this->*(KernelFunc))(x + epsilon * Eigen::Vector3d(0.0, 1.0, 0.0), fSmoothingLength)
		- (this->*(KernelFunc))(x - epsilon * Eigen::Vector3d(0.0, 1.0, 0.0), fSmoothingLength);

	vCentralDif[2] = (this->*(KernelFunc))(x + epsilon * Eigen::Vector3d(0.0, 0.0, 1.0), fSmoothingLength)
		- (this->*(KernelFunc))(x - epsilon * Eigen::Vector3d(0.0, 0.0, 1.0), fSmoothingLength);

	return (1 / 2 * epsilon) * vCentralDif;
}


Eigen::Vector3d SPHKernel::CubicSplineKernelGradient(const Eigen::Vector3d& x, double fSmoothingLength)
{
    double dAlpha = 3.0 / (2.0 * M_PI * fSmoothingLength * fSmoothingLength * fSmoothingLength);
    double relDist = x.norm() / fSmoothingLength;

    double derivative = 0.0;
    if (relDist < 1)
    {
        derivative = 0.5 * dAlpha * (3.0 * relDist - 4.0);
    }
    else if (relDist < 2)
    {
        derivative = -0.5 * pow((2.0 - relDist), 2.0);
    }
    return derivative * (x / (x.norm() * fSmoothingLength));
}

Eigen::Vector3d SPHKernel::QuinticSplineKernelGradient(const Eigen::Vector3d& x, double h)
{
    const double R = x.norm() / h;
    const double alpha_d = 3.0 / (359.0 * M_PI * pow(h, 3));

    double derivative = 0.0;
    if ((0.0 <= R) && (R < 1.0)) {
        derivative = -10.0 * alpha_d * R * (5.0 * pow(R, 3.0) - 12.0 * pow(R, 2.0) + 12.0);
    }
    else {
        if (R < 2.0) {
            derivative = 30.0 * alpha_d * pow(2.0 - R, 4.0) - 5.0 * alpha_d * pow(3 - R, 4.0);
        }
        else {
            if (R < 3.0) {
                derivative = -5.0 * alpha_d * pow(3.0 - R, 4.0);
            }
        }
    }
    return derivative * (x / (x.norm() * h));
}

Eigen::Vector3d SPHKernel::QuadraticSmoothingFunctionKernelGradient(const Eigen::Vector3d& x, double h)
{
    const double R = x.norm() / h;
    const double alpha_d = 5.0 / (4.0 * M_PI * pow(h, 3.0));

    assert(R <= 2.0 && "QuadraticSmoothingFunctionKernel: W(x, h) is not defined for R > 2");

    const double derivative = (3.0 / 8.0) * alpha_d * (R - 2.0);
    return derivative * (x / (x.norm() * h));
}

void SPHKernel::Run() {
	const Eigen::Vector3d x = Eigen::Vector3d(0.0, 0.0, 0.0) - Eigen::Vector3d(0.5, 0.5, 0.5);
	const double h = 1.0;

	printf("CubicSplineKernel(x, h) = %f\n", CubicSplineKernel(x, h));
	printf("QuinticSplineKernel(x, h) = %f\n", QuinticSplineKernel(x, h));
	printf("QuadraticSmoothingFunctionKernel(x, h) = %f\n", QuadraticSmoothingFunctionKernel(x, h));

	const double errorCubic =
		Eigen::Vector3d(CubicSplineKernelGradient(x, h) - ComputeCentralDifferences(&SPHKernel::CubicSplineKernel, x, h)).norm();
	const double errorQuintic =
		Eigen::Vector3d(QuinticSplineKernelGradient(x, h) - ComputeCentralDifferences(&SPHKernel::QuinticSplineKernel, x, h)).norm();
	const double errorQuadraticSmoothingFunction =
		Eigen::Vector3d(QuadraticSmoothingFunctionKernelGradient(x, h) - ComputeCentralDifferences(&SPHKernel::QuadraticSmoothingFunctionKernel, x, h)).norm();

	printf("cubic spline kernel gradient absolute error: %f\n", errorCubic);
	printf("quintic spline kernel gradient absolute error: %f\n", errorQuintic);
	printf("quadratic smoothing function kernel gradient absolute error: %f\n", errorQuadraticSmoothingFunction);

}
