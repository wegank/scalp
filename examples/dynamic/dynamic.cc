
#include <iostream>

#include <ILP/Solver.h>
#include <ILP/Exception.h>    // ILP::Exception
#include <ILP/SolverDynamic.h> // ILP::newSolverDynamic

// This example shows you, how to use the dynamic backend.
//
// It is a minimal version of simple_gurobi.
// The only relevant change is in line 24.

int main()
{
  try
  {
    // newSolverDynamic takes a list of solver-names.
    // The names correspond to the library name (libILP-NAME, e.g. "Gurobi" for libILP-Gurobi)
    // The solvers are searched at runtime from left to right.
    // In this example SCIP is prefered to Gurobi.
    // You can prepend additional solvers by defining the environment variable
    //   SCALP_SOLVER_LIST="name1;name2;..."
    // before you program starts.
    ILP::Solver s = ILP::Solver(ILP::newSolverDynamic({"CPLEX","SCIP","LPSolve"})); //"Gurobi",

    // disable solver output
    s.quiet=true;

    // enable presolving
    s.presolve=true;

    // print the used Solver
    std::cout << s.getBackendName() << std::endl;

    // set the timeout of the solver
    s.timeout = 1_hour + 30_minutes + 5_seconds;

    // declare the Variables
    ILP::Variable x = ILP::newIntegerVariable("x"); // x is free
    ILP::Variable y = ILP::newRealVariable("y",12.5,26);

    // Set the Objective
    s.setObjective(ILP::maximize(x-y));

    // add the Constraints
    s << (x+y<=31) << (5<=x<=30);
    
    s.writeLP("dynamic.lp");

    // Try to solve
    ILP::status stat = s.solve();

    // print results
    std::cout << "The result is " << stat << std::endl;
    std::cout << s.getResult() << std::endl;

  }
  catch(ILP::Exception &e)
  {
    std::cerr << "Error: " << e << std::endl;
  }

  return 0;
}
