
#include <iostream>

#include <ILP/Solver.h>
#include <ILP/Exception.h>    // ILP::Exception
#include <ILP/SolverGurobi.h> // ILP::newSolverGurobi

int main()
{
  try
  {
    ILP::Solver s = ILP::Solver(ILP::newSolverGurobi());
    s.quiet=true; // disable solver output

    // declare the Variables
    ILP::Variable x = ILP::newIntegerVariable("x"); // x is free
    // ILP::Variable x = ILP::newIntegerVariable("x",-ILP::INF(),ILP::INF()); // alternate
    ILP::Variable y = ILP::newRealVariable("y",12.5,26);

    // Set the Objective
    ILP::Term t = x;
    ILP::Objective o = ILP::maximize(t);
    s.setObjective(o); // alternate: s<<o;

    // print objective
    std::cout << "Objective: " << o << std::endl;

    // add the Constraints
    ILP::Constraint c1 = x+y<=30;
    ILP::Constraint c2 = 5<=x<=30;
    s<<c1<<c2;
    //s.addConstraint(c1); // alternate
    
    // write or print a LP-Format-representation
    //std::cout << s.showLP() << std::endl;
    s.writeLP("simple_gurobi.lp");

    // Try to solve
    ILP::status stat = s.solve();

    // print results
    std::cout << "The result is " << stat << std::endl;
    if(stat==ILP::status::OPTIMAL || stat==ILP::status::FEASIBLE)
    {
      ILP::Result r = s.getResult();
      std::cout << r << std::endl;

      //for(std::pair<const ILP::Variable,double> &p:r.values)
      //{
      //  std::cout << p.first << "=" << p.second << std::endl;
      //}
    }

  }
  catch(ILP::Exception &e)
  {
    std::cerr << "Error: " << e << std::endl;
  }

  return 0;
}
