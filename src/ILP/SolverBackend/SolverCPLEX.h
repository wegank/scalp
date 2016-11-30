#pragma once

#include <ILP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using CPLEX
  ILP::SolverBackend* newSolverCPLEX();
}

namespace ILP
{
  ILP::SolverBackend* newSolverCPLEX();
}
