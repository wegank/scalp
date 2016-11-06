#pragma once

#include <ILP/SolverBackend.h>

namespace ILP
{
  // create a new SolverBackend using SCIP / SoPlex
  ILP::SolverBackend* newSolverSCIP();
}
