
#include <ILP/SolverBackend/SolverSCIP.h>

bool ILP::SolverSCIP::addVariable(ILP::Variable v)
{
  (void)(v);
  return false;
}
bool ILP::SolverSCIP::addConstraint(ILP::Constraint cons)
{
  (void)(cons);
  return false;
}
bool ILP::SolverSCIP::setObjective(ILP::Objective o)
{
  (void)(o);
  return false;
}
ILP::status ILP::SolverSCIP::solve()
{
  return ILP::status::ERROR;
}
