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
	const double alpha_d = 3.0 / (359.0 * M_PI * pow(h, 3));

	assert(R <= 2.0 && "QuadraticSmoothingFunctionKernel: W(x, h) is not defined for R > 2");

	return alpha_d * ((3.0 / 16.0) * pow(R, 2.0) - (3.0 / 4.0) * R + (3.0 / 4.0));
}