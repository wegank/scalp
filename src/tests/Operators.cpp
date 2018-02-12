
#include <iostream>

#include <ScaLP/Solver.h>

int main(int argc, char** argv)
{
  // No solver given
  if(argc<2) return -1;

  ScaLP::Solver s{argv[1]};

  // print the name of the detected Solver in the log
  std::cout << s.getBackendName() << std::endl;

  ScaLP::Variable x = ScaLP::newIntegerVariable("x");
  ScaLP::Variable y = ScaLP::newIntegerVariable("y");
  ScaLP::Variable z = ScaLP::newIntegerVariable("z",0,100);

  // <= operators
  {
    s.presolve=false;
    s.setObjective(ScaLP::maximize(x+y+z));

    s << (x<=50) << (5<=y<=30) << (80<=z);

    ScaLP::status stat = s.solve();

    if(stat==ScaLP::status::OPTIMAL)
    {
      ScaLP::Result r = s.getResult();
      if(r.objectiveValue!=180 or r.values[x]!=50 or r.values[y]!=30 or r.values[z]!=100)
      {
        std::cout << "Operator <= not passed (wrong values)" << std::endl;
        return -1;

      }
      std::cout << "Operator <= passed." << std::endl;
    }
    else
    {
      std::cout << "Operator <= not passed (no optimal solution)" << std::endl;
      return -1;
    }
  }

  s.reset();

  // >= operators
  {
    s.presolve=false;
    s.setObjective(ScaLP::minimize(x+y+z));

    s << (x>=50) << (30>=y>=5) << (z>=80);

    ScaLP::status stat = s.solve();

    if(stat==ScaLP::status::OPTIMAL)
    {
      ScaLP::Result r = s.getResult();
      if(r.objectiveValue!=135 or r.values[x]!=50 or r.values[y]!=5 or r.values[z]!=80)
      {
        std::cout << "Operator >= not passed (wrong values)" << std::endl;
        return -1;
      }
      std::cout << "Operator >= passed." << std::endl;
    }
    else
    {
      std::cout << "Operator >= not passed (no optimal solution)" << std::endl;
      return -1;
    }
  }

  s.reset();

  // == operator
  {
    s.presolve=false;
    s.setObjective(ScaLP::minimize(x+y+z));

    s << (x==50) << (y==5) << (z==80);

    ScaLP::status stat = s.solve();

    if(stat==ScaLP::status::OPTIMAL)
    {
      ScaLP::Result r = s.getResult();
      if(r.objectiveValue!=135 or r.values[x]!=50 or r.values[y]!=5 or r.values[z]!=80)
      {
        std::cout << "Operator == not passed (wrong values)" << std::endl;
        return -1;
      }
      std::cout << "Operator == passed." << std::endl;
    }
    else
    {
      std::cout << "Operator == not passed (no optimal solution)" << std::endl;
      return -1;
    }
  }

  return 0;
}
