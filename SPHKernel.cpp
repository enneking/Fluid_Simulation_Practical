#include "SPHKernel.h"

#include "imgui.h"
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace SPH;

/** Cubic Spline Kernel */

CubicSplineKernel::CubicSplineKernel(double smoothingLength)
{
    SetSmoothingLength(smoothingLength);
}

void CubicSplineKernel::SetSmoothingLength(double smoothingLength)
{
    m_smoothingLength = smoothingLength;
    m_dAlpha = 3.0 / (2.0 * M_PI * pow(m_smoothingLength, 3.0));
}

double CubicSplineKernel::Evaluate(const Eigen::Vector3d& x)
{
	double relDist = x.norm() / m_smoothingLength;

    double mul = 0.0;
	if (relDist < 1)
	{
		mul = (2.0 / 3.0) - pow(relDist, 2.0) + 0.5 * pow(relDist, 3.0);
	}
	else if(relDist < 2)
	{
		mul = (1.0 / 6.0) * pow((2.0 - relDist) , 3.0);
	}
	else
	{
		mul = 0.0;
	}
    return mul * m_dAlpha;
}

Eigen::Vector3d CubicSplineKernel::EvaluateGradient(const Eigen::Vector3d& x)
{
    using namespace Eigen;
    double res_x, res_y, res_z;
    double h = m_smoothingLength;
    double r = x.norm();
    double R = r / h;
    double dAlpha = m_dAlpha;


    if ((0. <= R) && (R < 1.)) {
        res_x = (x.x() * (3.0*r - 4.0*h)) / (2 * h*h*h);
        res_y = (x.y() * (3.0*r - 4.0*h)) / (2 * h*h*h);
        res_z = (x.z() * (3.0*r - 4.0*h)) / (2 * h*h*h);
    }
    else if ((1. <= R) && (R < 2.)) {
        res_x = -(x.x() * (pow(r - 2.0*h, 2))) / (2.0*h*h*h*r);
        res_y = -(x.y() * (pow(r - 2.0*h, 2))) / (2.0*h*h*h*r);
        res_z = -(x.z() * (pow(r - 2.0*h, 2))) / (2.0*h*h*h*r);
    }
    else {
        res_x = res_y = res_z = 0.0;
    }
   
    return Eigen::Vector3d(res_x, res_y, res_z) * dAlpha;
}

/* Quintic Spline Kernel */

QuinticSplineKernel::QuinticSplineKernel(double smoothingLength)
{
    SetSmoothingLength(smoothingLength);
}

void QuinticSplineKernel::SetSmoothingLength(double smoothingLength)
{
    m_smoothingLength = smoothingLength;
    m_dAlpha = 3.0 / (359.0 * M_PI * pow(m_smoothingLength, 3));
}


double QuinticSplineKernel::Evaluate(const Eigen::Vector3d& x)
{
	const double R = x.norm() / m_smoothingLength;

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
	return m_dAlpha * mul;
}

Eigen::Vector3d QuinticSplineKernel::EvaluateGradient(const Eigen::Vector3d& x)
{
    const double R = x.norm() / m_smoothingLength;

    double derivative = 0.0;
    if ((0.0 <= R) && (R < 1.0)) {
        derivative = -10.0 * m_dAlpha * R * (5.0 * pow(R, 3.0) - 12.0 * pow(R, 2.0) + 12.0);
    }
    else {
        if (R < 2.0) {
            derivative = 30.0 * m_dAlpha * pow(2.0 - R, 4.0) - 5.0 * m_dAlpha * pow(3 - R, 4.0);
        }
        else {
            if (R < 3.0) {
                derivative = -5.0 * m_dAlpha * pow(3.0 - R, 4.0);
            }
        }
    }
    return derivative * (x / (x.norm() * m_smoothingLength));
}

/* Quadric Smoothing Function Kernel */


QuadricSmoothingFunctionKernel::QuadricSmoothingFunctionKernel(double smoothingLength)
{
    SetSmoothingLength(smoothingLength);
}

void QuadricSmoothingFunctionKernel::SetSmoothingLength(double smoothingLength)
{
    m_smoothingLength = smoothingLength;
    m_dAlpha = 5.0 / (4.0 * M_PI * pow(m_smoothingLength, 3.0));
}


double QuadricSmoothingFunctionKernel::Evaluate(const Eigen::Vector3d& x)
{
	const double R = x.norm() / m_smoothingLength;
	//assert(R <= 2.0 && "QuadricSmoothingFunctionKernel: W(x, h) is not defined for R > 2");
	return m_dAlpha * ((3.0 / 16.0) * pow(R, 2.0) - (3.0 / 4.0) * R + (3.0 / 4.0));
}

Eigen::Vector3d QuadricSmoothingFunctionKernel::EvaluateGradient(const Eigen::Vector3d& x)
{
    const double R = x.norm() / m_smoothingLength;
    //assert(R <= 2.0 && "QuadricSmoothingFunctionKernel: W(x, h) is not defined for R > 2");
    const double derivative = (3.0 / 8.0) * m_dAlpha * (R - 2.0);
    double res_x, res_y, res_z;
    res_x = derivative * (x.x() / (x.norm() * m_smoothingLength));
    res_y = derivative * (x.y() / (x.norm() * m_smoothingLength));
    res_z = derivative * (x.z() / (x.norm() * m_smoothingLength));
    return Eigen::Vector3d(res_x, res_y, res_z);
}

Eigen::Vector3d Kernel::ComputeCentralDifferences(Kernel* kernel, const Eigen::Vector3d& x)
{
	const double epsilon = 0.000001;
	Eigen::Vector3d vCentralDif;
	vCentralDif[0] = (kernel->Evaluate(x + epsilon * Eigen::Vector3d(1.0, 0.0, 0.0))
					- kernel->Evaluate(x - epsilon * Eigen::Vector3d(1.0, 0.0, 0.0)));

	vCentralDif[1] = (kernel->Evaluate(x + epsilon * Eigen::Vector3d(0.0, 1.0, 0.0))
		            - kernel->Evaluate(x - epsilon * Eigen::Vector3d(0.0, 1.0, 0.0)));

	vCentralDif[2] = (kernel->Evaluate(x + epsilon * Eigen::Vector3d(0.0, 0.0, 1.0))
		            - kernel->Evaluate(x - epsilon * Eigen::Vector3d(0.0, 0.0, 1.0)));

	return (1.0 / (2.0 * epsilon)) * vCentralDif;
}

