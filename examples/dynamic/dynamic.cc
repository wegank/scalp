
#include <iostream>

#include <ScaLP/Solver.h>
#include <ScaLP/Exception.h>    // ScaLP::Exception
#include <ScaLP/SolverDynamic.h> // ScaLP::newSolverDynamic

// This example shows you, how to use the dynamic backend.
//
// It is a minimal version of simple_gurobi.
// The only relevant change is in line 24.

int main()
{
  try
  {
    // newSolverDynamic takes a list of solver-names.
    // The names correspond to the library name (libScaLP-NAME, e.g. "Gurobi" for libScaLP-Gurobi)
    // The solvers are searched at runtime from left to right.
    // In this example SCIP is prefered to Gurobi.
    // You can prepend additional solvers by defining the environment variable
    //   SCALP_SOLVER_LIST="name1;name2;..."
    // before you program starts.
    ScaLP::Solver s = ScaLP::Solver(ScaLP::newSolverDynamic({"CPLEX","SCIP","LPSolve"})); //"Gurobi",

    // disable solver output
    s.quiet=true;

    // enable presolving
    s.presolve=true;

    // print the used Solver
    std::cout << s.getBackendName() << std::endl;

    // set the timeout of the solver
    s.timeout = 1_hour + 30_minutes + 5_seconds;

    // declare the Variables
    ScaLP::Variable x = ScaLP::newIntegerVariable("x"); // x is free
    ScaLP::Variable y = ScaLP::newRealVariable("y",12.5,26);

    // Set the Objective
    s.setObjective(ScaLP::maximize(x-y));

    // add the Constraints
    s << (x+y<=31) << (5<=x<=30);
    
    s.writeLP("dynamic.lp");

    // Try to solve
    ScaLP::status stat = s.solve();

    // print results
    std::cout << "The result is " << stat << std::endl;
    std::cout << s.getResult() << std::endl;

  }
  catch(ScaLP::Exception &e)
  {
    std::cerr << "Error: " << e << std::endl;
  }

  return 0;
}
