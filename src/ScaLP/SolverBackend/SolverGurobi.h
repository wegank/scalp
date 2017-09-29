#pragma once

#include <ScaLP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using Gurobi
  ScaLP::SolverBackend* newSolverGurobi();
}

namespace ScaLP
{
  ScaLP::SolverBackend* newSolverGurobi();
}
