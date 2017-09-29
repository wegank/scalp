
#include <iostream>

#include <ScaLP/Solver.h>
#include <ScaLP/Exception.h>    // ScaLP::Exception
#include <ScaLP/SolverGurobi.h> // ScaLP::newSolverGurobi

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
ScaLP::Objective createObjective(ScaLP::VariableMap &vs)
{
  // x1
  ScaLP::Term obj = V(x1);

  // x1 + x2
  obj+= V(x2);

  // 2 (x3 + x4)
  ScaLP::Term tmp = 2 * (V(x3) + V(x4));

  // (x1 + x2) + [ (2 x3 + 2 x4) + (2 x4) ]
  obj+= tmp + 2*V(x4);

  // max (x1 + x2 + 2 x3 + 4 x4)
  return ScaLP::maximize(obj);
}

// SUBJECT TO
//   x1 + x2 + 2*x3 <= 50
//   x1 + x3 + x4 <= 50
//   x1 + x2 + x3 <= 50
std::list<ScaLP::Constraint> createConstraints(ScaLP::VariableMap &vs)
{
  std::list<ScaLP::Constraint> ls;
  ScaLP::Term base = V(x1) + V(x2) + V(x3) + V(x4);

  ls.push_back(base - V(x4) + V(x3) <= 50);
  ls.push_back(base - V(x2) <= 50);
  ls.push_back(base - V(x4) <= 50);

  return ls;
}

int main()
{
  try
  {
    ScaLP::Solver s = ScaLP::Solver(ScaLP::newSolverGurobi());
    s.quiet=true; // disable solver output

    // create a bunch of the Variables
    ScaLP::VariableMap vs = 
    { { "x1", ScaLP::newIntegerVariable("x1",10,90) }
    , { "x2", ScaLP::newIntegerVariable("x2",0,ScaLP::INF()) }
    , { "x3", ScaLP::newIntegerVariable("x3",0,ScaLP::INF()) }
    , { "x4", ScaLP::newIntegerVariable("x4",0,ScaLP::INF()) }
    };

    // Set the Objective
    s.setObjective(createObjective(vs));

    // add the Constraints
    for(ScaLP::Constraint c:createConstraints(vs))
      s << c;

    std::cout << s.showLP() << std::endl;

    // Try to solve
    ScaLP::status stat = s.solve();

    // print results
    std::cout << "The result is " << stat << std::endl;
    if(stat==ScaLP::status::OPTIMAL || stat==ScaLP::status::FEASIBLE)
    {
      ScaLP::Result r = s.getResult();
      std::cout << r << std::endl;
    }

  }
  catch(ScaLP::Exception &e)
  {
    std::cerr << "Error: " << e << std::endl;
  }
  catch(std::out_of_range& e)
  {
    std::cerr << "Most likely a non-declared Variable was used:" << e.what() << std::endl;
  }

  return 0;
}
