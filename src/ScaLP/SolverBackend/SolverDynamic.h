#pragma once

#include <ScaLP/SolverBackend.h>

#include <string>
#include <list>

namespace ScaLP
{
  // create a new SolverBackend using the specified solvers (in that order)
  // The names correspond to the library name (libScaLP-NAME, e.g. "Gurobi" for libScaLP-Gurobi)
  // The solvers are searched at runtime from left to right.
  // You can prepend additional solvers by defining the environment variable
  //   SCALP_SOLVER_LIST="name1;name2;..."
  // before you program starts.
  ScaLP::SolverBackend* newSolverDynamic(std::list<std::string> lsa);

  // like above, but you need to specify the solvers via an environment-variable.
  // (SCALP_SOLVER_LIST)
  ScaLP::SolverBackend* newSolverDynamic();
}
