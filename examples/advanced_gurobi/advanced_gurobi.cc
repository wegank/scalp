
#include <iostream>

#include <ILP/Solver.h>
#include <ILP/Exception.h>    // ILP::Exception
#include <ILP/SolverGurobi.h> // ILP::newSolverGurobi

// This program demonstrates some advanced (but sometimes overly complicated used) usage.
//
// The encoded Problem:
//
// MAXIMIZE
//   x1 + x2 + 2 x3 + 4 x4
// SUBJECT TO
//   x1 + x2 + 2*x3 <= 50
//   x1 + x3 + x4 <= 50
//   x1 + x2 + x3 <= 50
// BOUNDS
//   10 <= x1 <= 90
// GENERALS
//   x1
//   x2
//   x3
//   x4
// END
//


// a shorthand-marcro
#define V(A) vs.at(#A)


// MAXIMIZE
//   x1 + x2 + 2 x3 + 4 x4
ILP::Objective createObjective(ILP::VariableMap &vs)
{
  // x1
  ILP::Term obj = V(x1);

  // x1 + x2
  obj+= V(x2);

  // 2 (x3 + x4)
  ILP::Term tmp = 2 * (V(x3) + V(x4));

  // (x1 + x2) + [ (2 x3 + 2 x4) + (2 x4) ]
  obj+= tmp + 2*V(x4);

  // max (x1 + x2 + 2 x3 + 4 x4)
  return ILP::maximize(obj);
}

// SUBJECT TO
//   x1 + x2 + 2*x3 <= 50
//   x1 + x3 + x4 <= 50
//   x1 + x2 + x3 <= 50
std::list<ILP::Constraint> createConstraints(ILP::VariableMap &vs)
{
  std::list<ILP::Constraint> ls;
  ILP::Term base = V(x1) + V(x2) + V(x3) + V(x4);

  ls.push_back(base - V(x4) + V(x3) <= 50);
  ls.push_back(base - V(x2) <= 50);
  ls.push_back(base - V(x4) <= 50);

  return ls;
}

int main()
{
  try
  {
    ILP::Solver s = ILP::Solver(ILP::newSolverGurobi());
    s.quiet=true; // disable solver output

    // create a bunch of the Variables
    ILP::VariableMap vs = 
    { { "x1", ILP::newIntegerVariable("x1",10,90) }
    , { "x2", ILP::newIntegerVariable("x2",0,ILP::INF()) }
    , { "x3", ILP::newIntegerVariable("x3",0,ILP::INF()) }
    , { "x4", ILP::newIntegerVariable("x4",0,ILP::INF()) }
    };

    // Set the Objective
    s.setObjective(createObjective(vs));

    // add the Constraints
    for(ILP::Constraint c:createConstraints(vs))
      s << c;

    std::cout << s.showLP() << std::endl;

    // Try to solve
    ILP::status stat = s.solve();

    // print results
    std::cout << "The result is " << stat << std::endl;
    if(stat==ILP::status::OPTIMAL || stat==ILP::status::FEASIBLE)
    {
      ILP::Result r = s.getResult();
      std::cout << r << std::endl;
    }

  }
  catch(ILP::Exception &e)
  {
    std::cerr << "Error: " << e << std::endl;
  }
  catch(std::out_of_range& e)
  {
    std::cerr << "Most likely a non-declared Variable was used:" << e.what() << std::endl;
  }

  return 0;
}
