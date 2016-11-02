#include <ILP/SolverBackend.h>

#include <ILP/Exception.h>

bool ILP::SolverBackend::addVariable(ILP::Variable v)
{
  (void)(v);
  return false;
}

bool ILP::SolverBackend::addVariables(ILP::VariableSet vs)
{
  for(const ILP::Variable v:vs)
  {
    if(!addVariable(v))
    {
      throw ILP::Exception("ILP: Can't add Variable \"" + v->name + "\" to the backend.");
      return false;
    }
  }
  return true;
}

bool ILP::SolverBackend::addConstraint(ILP::Constraint cons)
{
  (void)(cons);
  return false;
}

bool ILP::SolverBackend::addConstraints(std::list<ILP::Constraint> cons)
{
  for(const ILP::Constraint &c:cons)
  {
    if(!addConstraint(c))
    {
      throw ILP::Exception("ILP: Can't add Constraint to the backend.");
      return false;
    }
  }
  return true;
}

bool ILP::SolverBackend::setObjective(ILP::Objective o)
{
  (void)(o);
  return false;
}

ILP::status ILP::SolverBackend::solve()
{
  return ILP::status::ERROR;
}

void ILP::SolverBackend::reset()
{
  throw ILP::Exception("ILP: You need to implement the reset function in the backend.");
}

void ILP::SolverBackend::setConsoleOutput(bool verbose)
{
  (void)(verbose);
  throw ILP::Exception("ILP: You need to implement the setConsoleOutput function in the backend.");
}
