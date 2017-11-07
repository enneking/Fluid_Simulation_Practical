#include "SimSystem.h"

double QuinticSplineKernel(const Eigen::Vector3d& x, double h)
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

double QuadraticSmoothingFunctionKernel(const Eigen::Vector3d& x, double h)
{
    const double R = x.norm() / h;
    const double alpha_d = 3.0 / (359.0 * M_PI * pow(h, 3));

    assert(R <= 2.0 && "QuadraticSmoothingFunctionKernel: W(x, h) is not defined for R > 2");

    return alpha_d * ((3.0 / 16.0) * pow(R, 2.0) - (3.0 / 4.0) * R + (3.0 / 4.0));
}