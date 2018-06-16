
#include <ScaLP/Solver.h>

int main(int argc, char** argv)
{
  ScaLP::Variable x = ScaLP::newIntegerVariable("x");
  ScaLP::Variable y = ScaLP::newIntegerVariable("y");
  ScaLP::Variable z = ScaLP::newIntegerVariable("z");

  ScaLP::Term t;

  const int n = 50000;

  for(int i=0;i<n;++i)
  {
    ScaLP::Variable x = ScaLP::newIntegerVariable("x"+std::to_string(i));
    t+= 0*x;
  }
  for(int i=0;i<n;++i)
  {
    ScaLP::Variable x = ScaLP::newIntegerVariable("x"+std::to_string(i));
    t+= x*i;
    t-= x*i;
  }

  if(t.isConstant())
    return 0;
  else
    return -1;
}
