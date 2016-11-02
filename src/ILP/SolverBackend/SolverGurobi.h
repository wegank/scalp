#pragma once

#include <ILP/SolverBackend.h>

namespace ILP
{
  // create a new SolverBackend using Gurobi
  ILP::SolverBackend* newSolverGurobi();
}
