
#include <iostream>

#include "../ILP/Term.h"
#include "../ILP/Variable.h"
#include "../ILP/Solver.h"
#include "../ILP/SolverBackend/SolverDynamic.h"


int main(int argc, char** argv)
{
  // No solver given
  if(argc<2) return -1;

  ILP::Solver s{ILP::newSolverDynamic({argv[1]})};

  // print the name of the detected Solver in the log
  std::cout << s.getBackendName() << std::endl;

  // do something

  return -1; // 0 == passed
}
