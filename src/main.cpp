#include <iostream>
#include <string>
#include <utility>

#include "ScaLP/Exception.h"
#include "ScaLP/Solver.h"

#ifdef LP_PARSER
#include "parse.h"
#endif

void printHelp()
{
  std::cout << "Usage:" << std::endl << "  $ scalp [-s SolverName] file.lp" << std::endl;
}

std::pair<std::string,std::string> parseCommandLine(int argc, char** argv)
{
  if(argc<2)
  {
    std::cerr << "No arguments given" << std::endl;
    printHelp();
    return {"",""};
  }

  if(argc==2)
  { // lp-file only
    return {"",argv[1]};
  }
  
  if(argc==4)
  {
    if(std::string(argv[1])=="-s")
    { // solver and lp-file
      return {argv[2],argv[3]};
    }
  }

  std::cerr << "Wrong command line arguments" << std::endl;
  printHelp();
  return {"",""};
}

void printSolution(ScaLP::status stat, ScaLP::Result& result)
{
  std::cout << "The result is " << stat << std::endl;
  std::cout << result << std::endl;
}

int main(int argc, char** argv)
{

  auto conf = parseCommandLine(argc,argv);
  if(conf.second.empty()) return -1; // command line error (no file)

#ifdef LP_PARSER
  try
  {
    ScaLP::Solver s{conf.first};
    std::cout << s.getBackendName() << std::endl;

    // disable presolving for LPSolve
    s.presolve = s.getBackendName() != "Dynamic: LPSolve";

    // hide solver output
    s.quiet = true;

    s.load(conf.second);

    std::cout << s.showLP() << std::endl;

    ScaLP::status stat = s.solve();

    if(stat == ScaLP::status::OPTIMAL || stat == ScaLP::status::FEASIBLE)
    {
      auto res = s.getResult();

      printSolution(stat,res);
    }

    // TODO: functionality

  }
  catch(ScaLP::Exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }
  catch(std::exception& e)
  {
    std::cerr << "Unknown error: " << e.what() << std::endl;
  }
  catch(...)
  {
    std::cerr << "Unknown error: " << std::endl;
  }
#endif
  return 0;
}
