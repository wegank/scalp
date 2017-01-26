#pragma once

#include <ILP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using lp_solve
  ILP::SolverBackend* newSolverLPSolve();
}

namespace ILP
{
  ILP::SolverBackend* newSolverLPSolve();
}
