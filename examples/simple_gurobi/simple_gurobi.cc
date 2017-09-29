
#include <iostream>

#include <ScaLP/Solver.h>
#include <ScaLP/Exception.h>    // ScaLP::Exception
#include <ScaLP/SolverGurobi.h> // ScaLP::newSolverGurobi

int main()
{
  try
  {
    ScaLP::Solver s = ScaLP::Solver(ScaLP::newSolverGurobi());
    s.quiet=true; // disable solver output

    // declare the Variables
    ScaLP::Variable x = ScaLP::newIntegerVariable("x"); // x is free
    // ScaLP::Variable x = ScaLP::newIntegerVariable("x",-ScaLP::INF(),ScaLP::INF()); // alternate
    ScaLP::Variable y = ScaLP::newRealVariable("y",12.5,26);

    // Set the Objective
    ScaLP::Term t = x;
    ScaLP::Objective o = ScaLP::maximize(t);
    s.setObjective(o); // alternate: s<<o;

    // print objective
    std::cout << "Objective: " << o << std::endl;

    // add the Constraints
    ScaLP::Constraint c1 = x+y<=30;
    ScaLP::Constraint c2 = 5<=x<=30;
    s<<c1<<c2;
    //s.addConstraint(c1); // alternate
    
    // write or print a LP-Format-representation
    //std::cout << s.showLP() << std::endl;
    s.writeLP("simple_gurobi.lp");

    // Try to solve
    ScaLP::status stat = s.solve();

    // print results
    std::cout << "The result is " << stat << std::endl;
    if(stat==ScaLP::status::OPTIMAL || stat==ScaLP::status::FEASIBLE)
    {
      ScaLP::Result r = s.getResult();
      std::cout << r << std::endl;

      //for(std::pair<const ScaLP::Variable,double> &p:r.values)
      //{
      //  std::cout << p.first << "=" << p.second << std::endl;
      //}
    }

  }
  catch(ScaLP::Exception &e)
  {
    std::cerr << "Error: " << e << std::endl;
  }

  return 0;
}
