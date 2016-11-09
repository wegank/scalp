#pragma once

#include <ILP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using Gurobi
  ILP::SolverBackend* newSolverGurobi();
}

namespace ILP
{
  ILP::SolverBackend* newSolverGurobi();
}
