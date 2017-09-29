
#include <iostream>

#include "../ScaLP/Term.h"
#include "../ScaLP/Variable.h"
#include "../ScaLP/Solver.h"
#include "../ScaLP/SolverBackend/SolverDynamic.h"


int main(int argc, char** argv)
{
  // No solver given
  if(argc<2) return -1;

  ScaLP::Solver s{ScaLP::newSolverDynamic({argv[1]})};

  // print the name of the detected Solver in the log
  std::cout << s.getBackendName() << std::endl;

  // do something

  return -1; // 0 == passed
}
