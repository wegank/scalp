#pragma once

#include <ScaLP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using CPLEX
  ScaLP::SolverBackend* newSolverCPLEX();
}

namespace ScaLP
{
  ScaLP::SolverBackend* newSolverCPLEX();
}
