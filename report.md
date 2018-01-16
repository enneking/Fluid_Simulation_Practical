# SPH Fluid Simulation Practical - Report


### Contents

* Introduction:
    - Project specification
    - Phases of implementation (assignments)
    - Learning goals?

* Phase 1: Kernel implementation
    - Problems during implementation, especially with correctness of gradients!
    - Why didn't things work right off the bat? 
    - Parallel: setting up custom rendering, UI, etc

* Phase 2: Density estimation, Integration and Boundary Forces (and viscosity)
    - What went wrong during density estimation? (stiffness off by four magnitudes or something, general bouncy behaviour)
    - We thought it was the boundary forces being off
    - Testing with too few particles, yadda yadda
    - How did we fix it (do we know, really?)
    - Also: Neighborhood search, how did we implement it first (badly), what did we fix
    - How did viscosity play out

* Phase 3: Improved Boundary Handling, Position Based Pressure Solver
    - What went wrong with improved boundary handling?
    - Finding radius for particles based on boundary density!
    - same to fix normal boundary handling

* Phase 4: ...



## Introduction

### Project Specification

The goal of the project was to realize a fluid simulation based on *Smoothed Particle Hydrodynamics* (SPH), a method that approximates a continuous volume as a particle cloud, implementing the *Navier Stokes* equations via density estimation.

The basic idea behind SPH is to discretize the problem domain - a continuous fluid body - by approximating it using a finite set of particles. Each particle represents a part of the total volume. The goal of SPH is then to model volume preservation under various acting forces using a density estimation as a basis for pressure force exchange between particles as well as friction force computation. Additionally, rigidbodies and boundaries are introduced into the system as sets of locally static particles, with special handling of fluid-rigidbody coupling forces. 

The project was split into a few iterative steps, given out as bi-weekly assignments.
The first phase was the implementation of a set of kernel functions. These functions are used to *weigh* the contribution of neighboring particles when computing density, pressure and other values for each individual particle. 
The kernel functions are chosen such that they satisfy a set of requirements:

1)  Normalisation
2)  Symmetry
3)  Delta function property
4)  Non-negativity
5)  Compact support

The first assignment proposed three different functions that satisfy these requirements, as found in [REF]: The cubic spline function, the quintic spline function, and the quadric smooth function.
In our implementation, we mostly use the cubic spline function.

