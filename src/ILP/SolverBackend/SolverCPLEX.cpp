
#include <ILP/SolverBackend/SolverCPLEX.h>

bool ILP::SolverCPLEX::addVariable(ILP::Variable v)
{
  (void)(v);
  return false;
}
bool ILP::SolverCPLEX::addConstraint(ILP::Constraint cons)
{
  (void)(cons);
  return false;
}
bool ILP::SolverCPLEX::setObjective(ILP::Objective o)
{
  (void)(o);
  return false;
}
ILP::status ILP::SolverCPLEX::solve()
{
  return ILP::status::ERROR;
}
