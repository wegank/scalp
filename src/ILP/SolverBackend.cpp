#include <ILP/SolverBackend.h>

#include <iostream>

#include <ILP/Exception.h>

bool ILP::SolverBackend::addVariable(const ILP::Variable& v)
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
      throw ILP::Exception("Scalp: Can't add Variable \"" + v->name + "\" to the backend.");
      return false;
    }
  }
  return true;
}

bool ILP::SolverBackend::addConstraint(const ILP::Constraint& cons)
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
      throw ILP::Exception("Scalp: Can't add Constraint \"" + c.name + "\" to the backend.");
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
  throw ILP::Exception("Scalp: You need to implement the reset function in the backend.");
}

void ILP::SolverBackend::setConsoleOutput(bool verbose)
{
  (void)(verbose);
  throw ILP::Exception("Scalp: You need to implement the setConsoleOutput function in the backend.");
}

void ILP::SolverBackend::setTimeout(long t)
{
  (void)(t);
  throw ILP::Exception("Scalp: You need to implement the setTimeout function in the backend.");
}

void ILP::SolverBackend::presolve(bool presolve)
{
  (void)(presolve);
  std::cerr << "Scalp: presolve not supported by this backend, ignore this step." << std::endl;
}

void ILP::SolverBackend::setThreads(unsigned int t)
{
  (void)(t);
  std::cerr << "Scalp: a thread-limit is not supported by this backend, ignore it." << std::endl;
}
