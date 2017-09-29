#pragma once

#include <ScaLP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using lp_solve
  ScaLP::SolverBackend* newSolverLPSolve();
}

namespace ScaLP
{
  ScaLP::SolverBackend* newSolverLPSolve();
}
