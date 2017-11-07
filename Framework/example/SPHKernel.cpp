#include "SPHKernel.h"

SPHKernel::SPHKernel()
{
}

SPHKernel::~SPHKernel()
{
}

double SPHKernel::CubicSplineKernel(float& fDistance, float& fSmoothingLength)
{
	double dAlpha = 3.0 / (2.0 * M_PI * fSmoothingLength * fSmoothingLength * fSmoothingLength);
	double relDist = fDistance / fSmoothingLength;

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

