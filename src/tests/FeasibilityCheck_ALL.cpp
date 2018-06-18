
#include <ScaLP/Solver.h>

int main(int argc, char** argv)
{
  ScaLP::Variable x = ScaLP::newIntegerVariable("x");
  ScaLP::Variable y = ScaLP::newIntegerVariable("y");
  ScaLP::Variable z = ScaLP::newIntegerVariable("z");

  ScaLP::Term t = x+y+z;

  auto c1 = t <= 7;
  auto c2 = t >= 8;
  auto c3 = t == 6;
  auto c4 = 5 <= t <= 7;

  ScaLP::Result r;
  r.values =
  { {x,1}
  , {y,2}
  , {z,3}
  };

  if(c1.isFeasible(r) and not c2.isFeasible(r) and c3.isFeasible(r) and c4.isFeasible(r))
    return 0;
  else
    return -1;
}
