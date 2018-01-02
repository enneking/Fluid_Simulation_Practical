#pragma once

#include <Eigen/Core>

namespace SPH {

    class Kernel
    {
    public:
        virtual ~Kernel() = default;

        //void Run();

        virtual void    SetSmoothingLength(double smoothingLength) = 0;
        virtual double  Evaluate(const Eigen::Vector3d& x) = 0;
        virtual Eigen::Vector3d EvaluateGradient(const Eigen::Vector3d& x) = 0;

        static Eigen::Vector3d ComputeCentralDifferences(Kernel* kernel, const Eigen::Vector3d& x);

    };

    class CubicSplineKernel : public Kernel
    {
    public:
        CubicSplineKernel(double smoothingLength);
        virtual ~CubicSplineKernel() = default;

        //void Run();

        void    SetSmoothingLength(double smoothingLength) final override;
        double  Evaluate(const Eigen::Vector3d& x) final override;
        Eigen::Vector3d  EvaluateGradient(const Eigen::Vector3d& x) final override;

    private:
        double m_smoothingLength = 0.0;
        double m_dAlpha = 0.0;
    };

    class QuinticSplineKernel : public Kernel
    {
    public:
        QuinticSplineKernel(double smoothingLength);
        virtual ~QuinticSplineKernel() = default;

        //void Run();

        void    SetSmoothingLength(double smoothingLength) final override;
        double  Evaluate(const Eigen::Vector3d& x) final override;
        Eigen::Vector3d  EvaluateGradient(const Eigen::Vector3d& x) final override;
    private:
        double m_smoothingLength = 0.0;
        double m_dAlpha = 0.0;
    };

    class QuadricSmoothingFunctionKernel : public Kernel
    {
    public:
        QuadricSmoothingFunctionKernel(double smoothingLength);
        virtual ~QuadricSmoothingFunctionKernel() = default;

        //void Run();

        void    SetSmoothingLength(double smoothingLength) final override;
        double  Evaluate(const Eigen::Vector3d& x) final override;
        Eigen::Vector3d  EvaluateGradient(const Eigen::Vector3d& x) final override;

    private:
        double m_smoothingLength = 0.0;
        double m_dAlpha = 0.0;
    };
}
