#include "SPHKernel.h"

#include "imgui.h"
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

SPHKernel::SPHKernel()
{
}

SPHKernel::~SPHKernel()
{
}

double SPHKernel::CubicSplineKernel(const Eigen::Vector3d& x, double fSmoothingLength)
{
	double dAlpha = 3.0 / (2.0 * M_PI * pow(fSmoothingLength, 3.0));
	double relDist = x.norm() / fSmoothingLength;

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
    return mul * dAlpha;
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

double SPHKernel::QuadricSmoothingFunctionKernel(const Eigen::Vector3d& x, double h)
{
	const double R = x.norm() / h;
	const double alpha_d = 5.0 / (4.0 * M_PI * pow(h, 3.0));

	//assert(R <= 2.0 && "QuadricSmoothingFunctionKernel: W(x, h) is not defined for R > 2");

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

	return (1 / (2 * epsilon)) * vCentralDif;
}


Eigen::Vector3d SPHKernel::CubicSplineKernelGradient(const Eigen::Vector3d& x, double fSmoothingLength)
{
    double dAlpha = 3.0 / (2.0 * M_PI * pow(fSmoothingLength, 3.0));
    double relDist = x.norm() / fSmoothingLength;

    double derivative = 0.0;
    if (relDist < 1)
    {
		derivative = 2.0 * relDist + (3.0 / 2.0) * pow(relDist, 2.0);
    }
    else if (relDist < 2)
    {
        derivative = -0.5 * pow((2.0 - relDist), 2.0);
    }
    return derivative * dAlpha * (x / (x.norm() * fSmoothingLength));
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

Eigen::Vector3d SPHKernel::QuadricSmoothingFunctionKernelGradient(const Eigen::Vector3d& x, double h)
{
    const double R = x.norm() / h;
    const double alpha_d = 5.0 / (4.0 * M_PI * pow(h, 3.0));

    //assert(R <= 2.0 && "QuadricSmoothingFunctionKernel: W(x, h) is not defined for R > 2");

    const double derivative = (3.0 / 8.0) * alpha_d * (R - 2.0);
    return derivative * (x / (x.norm() * h));
}

void SPHKernel::Run() {

    static glm::vec3 x0(0.0f, 0.0f, 0.0f);
    static float h_ = 1.0;


    if (ImGui::Begin("SPHKernel Tests")) {
	    
        ImGui::DragFloat("h", &h_, 0.001f, 0.0f, 100.0f);
        const double h = (double)h_;

        ImGui::BeginChild("plots");

        static const int NUM_SAMPLES = 20;
        glm::vec3 x_range[NUM_SAMPLES];
        for (int i = 0; i < NUM_SAMPLES; ++i) {
            auto coords = glm::vec2(1.0f, 1.0f);
            coords = glm::normalize(coords) * (((float)i - (NUM_SAMPLES / 2.0f)) / NUM_SAMPLES);
            x_range[i] = glm::vec3(coords, 0.0f);
        }

        float dist[NUM_SAMPLES];
        for (auto i = 0; i < NUM_SAMPLES; ++i) {
            dist[i] = glm::distance(x_range[i], x0);
        }

        {   // cubic b spline plot
            double weights[NUM_SAMPLES];
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                const Eigen::Vector3d x(double(x_range[i].x - x0.x), double(x_range[i].y - x0.y), double(x_range[i].z - x0.z));
                weights[i] = CubicSplineKernel(x, h);
               
            }

            auto penPos = ImGui::GetCursorPos();
            ImGui::PlotHistogram("Cubic B Spline", [](void* data, int idx) -> float {
                auto w = (double*)data;
                return (float)w[idx];
            }, weights, NUM_SAMPLES, 0, nullptr, 0.0f, 20.0f, ImVec2(0, 200));

            /*ImGui::SetCursorPos(penPos);
           
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::PlotHistogram("Cubic B Spline distances", dist, NUM_SAMPLES, 0, nullptr, FLT_MIN, 1.0f, ImVec2(0, 100));
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();*/

        }

        {   // quintic kernel plot
            double weights[NUM_SAMPLES];
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                const Eigen::Vector3d x(double(x_range[i].x - x0.x), double(x_range[i].y - x0.y), double(x_range[i].z - x0.z));
                weights[i] = QuinticSplineKernel(x, h);
   
            }
            auto penPos = ImGui::GetCursorPos();
            ImGui::PlotHistogram("Quintic Spline", [](void* data, int idx) -> float {
                auto w = (double*)data;
                return (float)w[idx];
            }, weights, NUM_SAMPLES, 0, nullptr, 0.0f, 20.0f, ImVec2(0, 200));
        }

        {   // quadric 
            double weights[NUM_SAMPLES];
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                const Eigen::Vector3d x(double(x_range[i].x - x0.x), double(x_range[i].y - x0.y), double(x_range[i].z - x0.z));
                weights[i] = QuadricSmoothingFunctionKernel(x, h);
            }

            ImGui::PlotHistogram("Quadric Smoothing Function", [](void* data, int idx) -> float {
                auto w = (double*)data;
                return (float)w[idx];
            }, weights, NUM_SAMPLES, 0, nullptr, 0.0f, 20.0f, ImVec2(0, 200));
        }

        {   // cubic b spline derivative plot
            double error[NUM_SAMPLES];
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                const Eigen::Vector3d x(double(x_range[i].x - x0.x), double(x_range[i].y - x0.y), double(x_range[i].z - x0.z));
                error[i] = (CubicSplineKernelGradient(x, h) - ComputeCentralDifferences(&SPHKernel::CubicSplineKernel, x, h)).norm();
            }

            auto penPos = ImGui::GetCursorPos();
            ImGui::PlotHistogram("Cubic B Spline Derivative Error", [](void* data, int idx) -> float {
                auto w = (double*)data;
                return (float)w[idx];
            }, error, NUM_SAMPLES, 0, nullptr, FLT_MIN, FLT_MAX, ImVec2(0, 200));
        }

        {   // cubic b spline derivative plot
            double error[NUM_SAMPLES];
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                const Eigen::Vector3d x(double(x_range[i].x - x0.x), double(x_range[i].y - x0.y), double(x_range[i].z - x0.z));
                error[i] = (QuinticSplineKernelGradient(x, h) - ComputeCentralDifferences(&SPHKernel::QuinticSplineKernel, x, h)).norm();
            }

            auto penPos = ImGui::GetCursorPos();
            ImGui::PlotHistogram("Quintic Spline Derivative Error", [](void* data, int idx) -> float {
                auto w = (double*)data;
                return (float)w[idx];
            }, error, NUM_SAMPLES, 0, nullptr, FLT_MIN, FLT_MAX, ImVec2(0, 200));
        }
            
        {   // quadric derivative plot
            double error[NUM_SAMPLES];
            for (int i = 0; i < NUM_SAMPLES; ++i) {
                const Eigen::Vector3d x(double(x_range[i].x - x0.x), double(x_range[i].y - x0.y), double(x_range[i].z - x0.z));
                error[i] = (QuadricSmoothingFunctionKernelGradient(x, h) - ComputeCentralDifferences(&SPHKernel::QuadricSmoothingFunctionKernel, x, h)).norm();
            }

            auto penPos = ImGui::GetCursorPos();
            ImGui::PlotHistogram("Quadric Smoothing Function Derivative Error", [](void* data, int idx) -> float {
                auto w = (double*)data;
                return (float)w[idx];
            }, error, NUM_SAMPLES, 0, nullptr, FLT_MIN, FLT_MAX, ImVec2(0, 200));
        }

        ImGui::EndChild();
    } ImGui::End();
}
