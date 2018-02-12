
#include <iostream>

#include <ScaLP/Solver.h>

int main(int argc, char** argv)
{
  // No solver given
  if(argc<2) return -1;

  ScaLP::Solver s{argv[1]};

  // print the name of the detected Solver in the log
  std::cout << s.getBackendName() << std::endl;

  // do something

  return 0; // -1 == not passed
}
