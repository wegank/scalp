#pragma once

#include <ILP/SolverBackend.h>

extern "C"
{
  // create a new SolverBackend using SCIP / SoPlex
  ILP::SolverBackend* newSolverSCIP();
}

namespace ILP
{
  ILP::SolverBackend* newSolverSCIP();
}
