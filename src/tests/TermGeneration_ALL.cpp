
#include <ScaLP/Solver.h>

int main(int argc, char** argv)
{
  ScaLP::Variable x = ScaLP::newIntegerVariable("x");
  ScaLP::Variable y = ScaLP::newIntegerVariable("y");
  ScaLP::Variable z = ScaLP::newIntegerVariable("z");

  // TODO: add more
  ScaLP::Term t = 1*x-x+2*x-(2*x-x)+y+z;
  ScaLP::Term r = x+y+z;

  if(t==r)
    return 0;
  else
    return -1;
}
