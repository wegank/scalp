
#include <ILP/Term.h>
#include <ILP/Variable.h>
#include <ILP/Solver.h>


int main(int argc, char** argv)
{
  ILP::Variable x = ILP::newIntegerVariable("x");
  ILP::Variable y = ILP::newIntegerVariable("y");
  ILP::Variable z = ILP::newIntegerVariable("z");

  // TODO: add more
  ILP::Term t = 1*x-x+2*x-(2*x-x)+y+z;
  ILP::Term r = x+y+z;

  if(t==r)
    return 0;
  else
    return -1;
}
