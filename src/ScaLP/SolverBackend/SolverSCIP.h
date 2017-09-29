#pragma once

#include <ScaLP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using SCIP / SoPlex
  ScaLP::SolverBackend* newSolverSCIP();
}

namespace ScaLP
{
  ScaLP::SolverBackend* newSolverSCIP();
}
