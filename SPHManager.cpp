#include "SPHManager.h"
#include "imgui.h"


SPHManager::SPHManager()
{
}


SPHManager::~SPHManager()
{
    
}


double SPHManager::EvaluateConstraint(size_t idx)
{
    auto val = glm::max((state.density[idx] / settings.restDensity) - 1.0, 0.0);
    if (val > 1.0) { std::cout << "C = " << val << std::endl; }
    return val;
}

Eigen::Vector3d SPHManager::EvaluateConstraintDerivativeJ(size_t i, size_t j)
{
    auto const mass = m_oParticleManager.GetParticleMass();

    auto const xi = m_oParticleManager.GetParticlePositions()->at(i);
    auto const xj = m_oParticleManager.GetParticlePositions()->at(j);

    return -((mass / settings.restDensity) * m_pSPHKernel->EvaluateGradient(xi - xj));
}

Eigen::Vector3d SPHManager::EvaluateConstraintDerivativeI(size_t idx)
{
    auto const mass = m_oParticleManager.GetParticleMass();
    auto const xi = m_oParticleManager.GetParticlePositions()->at(idx);

    Eigen::Vector3d accum = Eigen::Vector3d(0.0, 0.0, 0.0);
    for (size_t j = 0; j < m_oParticleManager.GetParticleContainer()->size(); ++j) {
        // if(j == idx) continue;
        auto const xj = m_oParticleManager.GetParticlePositions()->at(j);
        accum += mass * m_pSPHKernel->EvaluateGradient(xi - xj);
    }
    return accum * (1.0 / settings.restDensity);
}

void SPHManager::ComputeLambdas(WorkGroup* workGroup)
{
    const auto firstParticle = workGroup->particleOffset;
    const auto numParticles = workGroup->particleCount;
    const auto lastParticle = numParticles + firstParticle;

    for (auto idx = firstParticle; idx < lastParticle; ++idx) {
        static const double epsilon = 10e-4;
        double accum = 0.0;

        auto& discretizations = m_oCompactNSearch->discretizations();
        auto numNeighbors = discretizations[m_fluidDiscretizationId].n_neighbors(idx);

        for (int n = 0; n < numNeighbors; ++n) {

            auto nID = discretizations[m_fluidDiscretizationId].neighbor(idx, n);
            auto j = nID.index;
            if (nID.object_id != m_fluidDiscretizationId) continue;
            // if(j == idx) continue;
            accum += EvaluateConstraintDerivativeJ(idx, j).squaredNorm();
        }
        state.lambda[idx] = -(EvaluateConstraint(idx) / ((accum) + epsilon));
    }
}

Eigen::Vector3d SPHManager::ComputeDisplacement(size_t idx)
{
    auto const mass = m_oParticleManager.GetParticleMass();
    auto const xi = m_oParticleManager.GetParticlePositions()->at(idx);

    auto const lambda_i = state.lambda[idx];
    Eigen::Vector3d accum = Eigen::Vector3d(0.0, 0.0, 0.0);

    auto& discretizations = m_oCompactNSearch->discretizations();
    auto numNeighbors = discretizations[m_fluidDiscretizationId].n_neighbors(idx);
    
    for (int n = 0; n < numNeighbors; ++n) {

        auto nID = discretizations[m_fluidDiscretizationId].neighbor(idx, n);
        auto j = nID.index;
        if (nID.object_id != m_fluidDiscretizationId) continue;
        // if(j == idx) continue;
        auto const xj = m_oParticleManager.GetParticlePositions()->at(j);
        auto const lambda_j = state.lambda[j];

        accum += mass * (lambda_i + lambda_j) * m_pSPHKernel->EvaluateGradient(xi - xj);
    }
    return (1.0 / settings.restDensity) * accum;
}

ParticleManager* SPHManager::GetParticleManager()
{
	return &m_oParticleManager;
}

void SPHManager::Init()
{
    /*
    for (size_t i = 0; i < 1024; ++i) {
        auto posX = (i / 1024.0) * 4.0 - 2;
        m_oParticleManager.AddParticle(Eigen::Vector3d(posX, 0.0, 0.0));
    }
    */

    m_oParticleManager.InitBuffers();
    settings.smoothingLength = m_oParticleManager.m_fParticleRadius * 4.0;
 
    m_oCompactNSearch = std::make_unique<CompactNSearch>(settings.smoothingLength * 2.0f);
    m_fluidDiscretizationId = m_oCompactNSearch->add_discretization(
        &(*m_oParticleManager.GetParticlePositions())[0],
        m_oParticleManager.GetParticleContainer()->size(),
        true, true);
    m_boundaryDiscretizationId = m_oCompactNSearch->add_discretization(
        m_oParticleManager.GetBoundaryPositions(),
        m_oParticleManager.GetBoundaryParticleCount(),
        false, false);

    for (auto i = 0; i < m_oParticleManager.GetParticleContainer()->size(); ++i) {
        m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity = Eigen::Vector3d(0.0, 0.0, 0.0);
    }

    // @TODO look into why quintic and quadric smoothing function dont work well
    m_pSPHKernel = new SPH::CubicSplineKernel(settings.smoothingLength);

    double relError = 0.0;
    for (int i = 0; i < 10; ++i) {
        auto x = Eigen::Vector3d(1.0, 0.0, 0.0) * (((double)i + 1.0) / 10.0);
        auto a = m_pSPHKernel->EvaluateGradient(x);
        auto b = SPH::Kernel::ComputeCentralDifferences(m_pSPHKernel, x);
        relError += (a - b).norm() / (a.norm());
    }
    relError /= 10.0;
    printf("Avg. relative Error is %.12f\n", relError);

    // initialize threads and work groups
    const auto numParticles = m_oParticleManager.GetParticleContainer()->size();
    state.boundaryForce.resize(numParticles, Eigen::Vector3d(0.0, 0.0, 0.0));
    state.pressure.resize(numParticles, 0.0);
    state.density.resize(numParticles, 0.0);
    state.lambda.resize(numParticles, 0.0);

    // precompute volume estimation function for boundaries
    {
        double smoothingLength = settings.smoothingLength; // @TODO compute that smoothing length from boundary resolution because that's important
        CompactNSearch search(smoothingLength);
        auto discrId = search.add_discretization(
            m_oParticleManager.GetBoundaryPositions(),
            m_oParticleManager.GetBoundaryParticleCount(),
            false, true);
        search.neighborhood_search();
        auto& boundaryDiscretization = search.discretizations()[discrId];
        state.boundaryD.resize(m_oParticleManager.GetBoundaryParticleCount(), 0.0);
        auto boundaryKernel = SPH::CubicSplineKernel(smoothingLength);
        for (size_t i = 0; i < m_oParticleManager.GetBoundaryParticleCount(); ++i) {
            auto x_i = m_oParticleManager.GetBoundaryPositions()[i];
            double t = 0.0;
            for (int j = 0; j < boundaryDiscretization.n_neighbors(i); ++j) {
                auto k = boundaryDiscretization.neighbor(i, j).index;
                if (boundaryDiscretization.neighbor(i, j).object_id != discrId) continue;
                //if (k == i) continue;
                auto x_k = m_oParticleManager.GetBoundaryPositions()[k];
                double weight = boundaryKernel.Evaluate(x_i - x_k);
                t += weight;
            }
            state.boundaryD[i] = settings.restDensity / t;
        }
    }

    
    const auto numThreads = m_numThreads = std::thread::hardware_concurrency() < MAX_THREADS ? std::thread::hardware_concurrency() : MAX_THREADS;

    size_t particleOffset = 0;
    size_t numDistributedParticles = 0;

    for (auto i = 0; i < numThreads - 1; ++i) {
        auto context = &m_threadContext[i];
        context->id = i;
        //context->pool = &m_threadPool;
        context->sph = this;

        size_t particleCount = (numParticles / numThreads) <= (numParticles - numDistributedParticles) ? (numParticles / numThreads) : (numParticles - numDistributedParticles);
        context->workGroup = WorkGroup(particleOffset, particleCount);
        particleOffset += particleCount;
        numDistributedParticles += particleCount;

        //m_threadPool.threads.emplace_back(new std::thread([context]() {
        //    do {
        //        std::unique_lock<std::mutex> lock;
        //        context->pool->signal.wait(lock);
        //        //context->sph->UpdateWorkGroup(&context->workGroup, context->sph->m_dt);
        //        context->sph->m_openWorkCounter--;
        //        while(context->sph->m_openWorkCounter > 0) {}
        //    } while (!context->pool->exit);
        //}));
    }
    {
        auto context = &m_threadContext[MAX_THREADS - 1];
        context->id = MAX_THREADS - 1;
        //context->pool = &m_threadPool;
        context->sph = this;

        size_t particleCount = (numParticles - numDistributedParticles);
        context->workGroup = WorkGroup(particleOffset, particleCount);
        particleOffset += particleCount;
        numDistributedParticles += particleCount;
    }
    assert(numDistributedParticles == numParticles);
    printf("Haha\n");
}


void SPHManager::Update(double dt)
{
    m_dt = dt;
    // neighborhood computation
    m_oCompactNSearch->neighborhood_search();

    auto numParticles = m_oParticleManager.GetParticleContainer()->size();

    for (size_t i = 0; i < numParticles; ++i) {
        state.boundaryForce[i] = Eigen::Vector3d(0.0, 0.0, 0.0);
        state.pressure[i]= 0.0;
        state.density[i] = 0.0;
    }
    
    auto numThreads = m_numThreads;
    m_openPressureCounter = numThreads;
    m_openBoundaryForceCounter = numThreads;
    m_openIntegrationCounter = numThreads;
    m_openWorkCounter = numThreads;
  
    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads - 1; ++i) {
        
        auto context = &m_threadContext[i];
        threads.emplace_back([context]() {
            context->sph->UpdateWorkGroup(&context->workGroup, context->sph->m_dt);
        });
    }

    UpdateWorkGroup(&m_threadContext[numThreads - 1].workGroup, dt);
    
    for (auto& thread : threads) { thread.join();  }

    //while (m_openWorkCounter > 0) {}
}


void SPHManager::UpdateWorkGroup(WorkGroup* workGroup, double dt)
{
    const auto firstParticle = workGroup->particleOffset;
    const auto numParticles = workGroup->particleCount;
    const auto lastParticle = numParticles + firstParticle;
    //assert(numParticles != 0);
    if (numParticles == 0) { 
        m_openBoundaryForceCounter--;
        m_openPressureCounter--;
        m_openIntegrationCounter--;
        m_openWorkCounter--;
        return; 
    }

    auto density = &state.density[0];
    auto pressure = &state.pressure[0];
    auto boundaryD = &state.boundaryD[0];

    double mass = m_oParticleManager.GetParticleMass();

    const double stiffness = settings.stiffness;
    const double p0 = settings.restDensity;    // rest density
    const double smoothingLength = settings.smoothingLength;

    std::vector<Eigen::Vector3d> displacement(workGroup->particleCount);

    // get precomputed neighborhoods
    auto& discretizations = m_oCompactNSearch->discretizations();

    //// density + pressure computation
    for (int i = firstParticle; i < lastParticle; ++i) {

        auto x_i = m_oParticleManager.GetParticlePositions()->at(i);

        // get neighbours
        auto numNeighbors = discretizations[m_fluidDiscretizationId].n_neighbors(i);

        
        for (int j = 0; j < numNeighbors; ++j) {

            auto nID = discretizations[m_fluidDiscretizationId].neighbor(i, j);
            auto nIndex = nID.index;

            if (nID.object_id != m_fluidDiscretizationId) {
                if (!settings.useImprovedBoundaryHandling) { continue; }  
                auto x_j = m_oParticleManager.GetBoundaryPositions()[nIndex];
                double weight = m_pSPHKernel->Evaluate(x_i - x_j);
                density[i] += boundaryD[nIndex] * weight;
            }
            else {
                auto x_j = m_oParticleManager.GetParticlePositions()->at(nIndex);
                double weight = m_pSPHKernel->Evaluate(x_i - x_j);
                density[i] += mass * weight;
            }
        }
        pressure[i] = glm::max(stiffness * (density[i] - p0), 0.0);
    }

    {   // wait for other work groups to reach this stage
        m_openPressureCounter--;
        while (m_openPressureCounter > 0) {}
    }

    // boundary force computation
    auto boundaryForce = &state.boundaryForce[0];
    for (int i = firstParticle; i < lastParticle; ++i) {
        auto x_i = m_oParticleManager.GetParticlePositions()->at(i);

        auto boundaryPositions = m_oParticleManager.GetBoundaryPositions();
        const double h = 0.1;  // @TODO work this out, too
        const double bR = 88.0;
        const double boundaryMass = 1.0;   
        const double fluidMass = m_oParticleManager.GetParticleMass();

        auto v_i = m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity;

        auto numNeighbors = discretizations[m_fluidDiscretizationId].n_neighbors(i);
        for (int j = 0; j < numNeighbors; ++j) {

            auto nID = discretizations[m_fluidDiscretizationId].neighbor(i, j);
            auto nIndex = nID.index;
            if (nID.object_id != m_boundaryDiscretizationId) continue;
            auto k = nIndex;

            //if (k < numParticles) { continue; }
            //k -= numParticles;

            auto x_k = boundaryPositions[k];

            auto f = Eigen::Vector3d(0.0, 0.0, 0.0);
            if (!settings.useImprovedBoundaryHandling) {
                auto diff = x_i - x_k;
                auto dist = diff.norm();
                auto alpha = bR / dist;
                auto relMass = boundaryMass / (fluidMass + boundaryMass);

                auto q = dist / h;

                //double x = 0.0;
                //auto a = (1 - x / h) ? x < h : 0.0;

                auto tau = (bR / dist);
                if (0 < q && q < (2.0 / 3.0)) {
                    tau *= (2.0 / 3.0);
                }
                else if ((2.0 / 3.0) < q && q < 1.0) {
                    tau *= (2.0 * q - (3.0 / 2.0) * (q * q));
                }
                else if (1.0 < q && q < 2.0) {
                    tau *= 0.5 * (2.0 - q) * (2.0 - q);
                }
                else {
                    tau *= 0.0;
                }
                f = relMass * tau * (diff / dist); // *(-diff.normalized().dot(v.normalized()) > 0.0 ? 1.0 : 0.0);

                {   // viscosity / friction (assignment04-1)
                    const double alpha = 0.2f;    // viscosity coefficient
                    const double epsilon = 0.01;  // regularization parameter
                    const double h = settings.smoothingLength;

                    const auto xDiff = x_i - x_k;
                    const auto vDiff = v_i;       // boundary particle has zero velocity

                    auto v = (alpha * h) / (2 * state.density[i]);
                    const double pi = -v * (std::min(vDiff.dot(xDiff), 0.0) / std::abs((xDiff.dot(xDiff) + epsilon * h * h)));
                    f += -mass * state.boundaryD[k] * pi * m_pSPHKernel->EvaluateGradient(x_i - x_k);
                }
            }
            else {
                f = -(fluidMass * boundaryD[k]) * (pressure[i] / (density[i] * density[i])) * m_pSPHKernel->EvaluateGradient(x_i - x_k);
            }
            boundaryForce[i] += f;
        }
    }

    {   // wait for other work groups to reach this stage
        m_openBoundaryForceCounter--;
        while (m_openBoundaryForceCounter > 0) {}
    }

    // pressure force calculation and integration
    for (int i = firstParticle; i < lastParticle; ++i) {
        auto x_i = m_oParticleManager.GetParticlePositions()->at(i);

        auto accel = Eigen::Vector3d(0.0, 0.0, 0.0);

        if (!settings.usePressureSolver) {  // not using the pressure solver
            auto numNeighbors = discretizations[m_fluidDiscretizationId].n_neighbors(i);
            for (int j = 0; j < numNeighbors; ++j) {

                auto nID = discretizations[m_fluidDiscretizationId].neighbor(i, j);
                auto nIndex = nID.index;
                if (nID.object_id != m_fluidDiscretizationId) continue;

                auto x_j = m_oParticleManager.GetParticlePositions()->at(nIndex);
                auto weight = m_pSPHKernel->EvaluateGradient(x_i - x_j);
                accel += mass * ((pressure[i] / (density[i] * density[i])) + (pressure[nIndex] / (density[nIndex] * density[nIndex]))) * weight;
            }
            accel = -accel;
        }

        accel += Eigen::Vector3d(0.0, -9.81, 0.0);
        accel += (1.0 / mass) * boundaryForce[i];

        m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity += accel * dt;
        auto velocity = m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity;
        displacement[i - workGroup->particleOffset] = velocity * dt;
    }

    if(settings.usePressureSolver) {   // pressure solver iterations
        for (size_t it = 0; it < settings.numSolverIterations; ++it) {
            ComputeLambdas(workGroup);

            for (auto i = firstParticle; i < lastParticle; ++i) {
                displacement[i - workGroup->particleOffset] += ComputeDisplacement(i);
            }
        }
        for (auto i = firstParticle; i < lastParticle; ++i) {
            auto newX = m_oParticleManager.GetParticlePositions()->at(i) + displacement[i - workGroup->particleOffset];
            m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity = (1.0 / dt) * (newX - m_oParticleManager.GetParticlePositions()->at(i));
        }
    }

    {   // wait for other work groups to reach this stage
        m_openIntegrationCounter--;
        while(m_openIntegrationCounter > 0) {}
    }

    // viscosity
    const double e = 0.05;
    for (int i = firstParticle; i < lastParticle; ++i) {
        auto vPrime = Eigen::Vector3d(0.0, 0.0, 0.0);

        auto numNeighbors = discretizations[m_fluidDiscretizationId].n_neighbors(i);
        for (int j = 0; j < numNeighbors; ++j) {

            auto nID = discretizations[m_fluidDiscretizationId].neighbor(i, j);
            auto nIndex = nID.index;
            if (nID.object_id != m_fluidDiscretizationId) continue;

            auto v_i = m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity;
            auto v_j = m_oParticleManager.GetParticleContainer()->at(nIndex).m_vVelocity;

            auto velDiff = (v_j - v_i);

            auto x_i = m_oParticleManager.GetParticlePositions()->at(i);
            auto x_j = m_oParticleManager.GetParticlePositions()->at(nIndex);
            auto weight = m_pSPHKernel->Evaluate(x_i - x_j);

            vPrime += (mass / density[nIndex]) * velDiff * weight;
        }
        m_oParticleManager.GetParticleContainer()->at(i).m_vVelocity += e * vPrime;


        // position update
        m_oParticleManager.GetParticlePositions()->at(i) += displacement[i - workGroup->particleOffset];
    }


    {   // work is done
        m_openWorkCounter--;
    }
}

static double g_avgDensity = 0.0;
void SPHManager::GUI()
{

    if (ImGui::Begin("SPHManager")) {
        struct DensityPlotData
        {
            SPHManager* self = nullptr;
        } densityPlotData = { this };
        ImGui::PlotHistogram("Density Samples", [](void* data, int idx) -> float {
            auto densityPlotData = static_cast<DensityPlotData*>(data);
            auto density = densityPlotData->self->GetDensityWithIndex(idx);
            return (float)density;
        }, &densityPlotData, m_oParticleManager.GetParticleContainer()->size(), /*m_oParticleManager.GetParticleContainer()->size() - 1024*/0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 200));
        g_avgDensity = 0.0;
        for (size_t i = 0; i < m_oParticleManager.GetParticleContainer()->size(); ++i) {
            g_avgDensity += state.density[i];
        }
        g_avgDensity /= m_oParticleManager.GetParticleContainer()->size();
        ImGui::Text("Average density: %f", g_avgDensity);

        
        ImGui::PlotHistogram("Lambda Samples", [](void* data, int idx) -> float {
            auto densityPlotData = static_cast<DensityPlotData*>(data);
            auto lambda = densityPlotData->self->state.lambda[idx];
            return (float)lambda;
        }, &densityPlotData, m_oParticleManager.GetParticleContainer()->size(), /*m_oParticleManager.GetParticleContainer()->size() - 1024*/0, nullptr, 0.0f, FLT_MAX, ImVec2(0, 200));
        double g_avgLambda = 0.0;
        static double g_highestLambda = 0.0;
        for (size_t i = 0; i < m_oParticleManager.GetParticleContainer()->size(); ++i) {
            g_avgLambda += state.lambda[i];
            if (std::abs(state.lambda[i]) > std::abs(g_highestLambda)) {
                g_highestLambda = state.lambda[i];
            }
        }
        g_avgLambda /= m_oParticleManager.GetParticleContainer()->size();
        ImGui::Text("Average lambda: %f", g_avgLambda);
        ImGui::Text("Highest lambda: %f", g_highestLambda);

        ImGui::PlotHistogram("Boundary Forces", [](void* data, int idx) -> float {
            Eigen::Vector3d* forces = static_cast<Eigen::Vector3d*>(data);
            return (float)forces[idx].norm();
        }, state.boundaryForce.data(), state.boundaryForce.size());

        if (ImGui::TreeNode("Settings")) {

            float gravity = (float)settings.gravityForce;
            float stiffness = (float)settings.stiffness;
            //float simSpeed = (float)settings.simSpeed;
            float restDens = (float)settings.restDensity;
            float mass = (float)m_oParticleManager.GetParticleMass();
            //float radius = (float)settings.particleRadius;
            float smoothingLength = (float)settings.smoothingLength;

            ImGui::DragFloat("Gravity", &gravity, 0.01f, -10.0f, 10.0f);
            ImGui::DragFloat("Stiffness", &stiffness, 1.0f);
            //ImGui::DragFloat("Simulation Speed", &simSpeed, 0.01f, 0.0f, 2.0f);
            ImGui::DragFloat("Rest Density", &restDens, 0.01f);
            ImGui::DragFloat("Mass", &mass, 0.01f);
            //ImGui::DragFloat("Kernel Radius", &radius, 0.01);
            ImGui::DragFloat("Smoothing Length", &smoothingLength, 0.01f, 0.1f, 1.0f);
            ImGui::Checkbox("Use improved boundary handling", &settings.useImprovedBoundaryHandling);

            settings.gravityForce = (double)gravity;
            settings.stiffness = (double)stiffness;
            //settings.simSpeed = (double)simSpeed;
            settings.restDensity = (double)restDens;
            m_oParticleManager.SetParticleMass((double)mass);
            settings.smoothingLength = smoothingLength;

            //settings.particleRadius = (double)radius;
            //m_oCompactNSearch->set_radius(settings.particleRadius);

            ImGui::TreePop();

        }

    } ImGui::End();
}
