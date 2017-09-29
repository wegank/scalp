#include <ScaLP/SolverBackend.h>

#include <iostream>

#include <ScaLP/Exception.h>

bool ScaLP::SolverBackend::addVariable(const ScaLP::Variable& v)
{
  (void)(v);
  return false;
}

bool ScaLP::SolverBackend::addVariables(ScaLP::VariableSet vs)
{
  for(const ScaLP::Variable v:vs)
  {
    if(!addVariable(v))
    {
      throw ScaLP::Exception("Scalp: Can't add Variable \"" + v->getName() + "\" to the backend.");
      return false;
    }
  }
  return true;
}

bool ScaLP::SolverBackend::addConstraint(const ScaLP::Constraint& cons)
{
  (void)(cons);
  return false;
}

bool ScaLP::SolverBackend::addConstraints(std::list<ScaLP::Constraint> cons)
{
  for(const ScaLP::Constraint &c:cons)
  {
    if(!addConstraint(c))
    {
      throw ScaLP::Exception("Scalp: Can't add Constraint \"" + c.name + "\" to the backend.");
      return false;
    }
  }
  return true;
}

bool ScaLP::SolverBackend::setObjective(ScaLP::Objective o)
{
  (void)(o);
  return false;
}

ScaLP::status ScaLP::SolverBackend::solve()
{
  return ScaLP::status::ERROR;
}

void ScaLP::SolverBackend::reset()
{
  throw ScaLP::Exception("Scalp: You need to implement the reset function in the backend.");
}

void ScaLP::SolverBackend::setConsoleOutput(bool verbose)
{
  (void)(verbose);
  throw ScaLP::Exception("Scalp: You need to implement the setConsoleOutput function in the backend.");
}

void ScaLP::SolverBackend::setTimeout(long t)
{
  (void)(t);
  throw ScaLP::Exception("Scalp: You need to implement the setTimeout function in the backend.");
}

void ScaLP::SolverBackend::presolve(bool presolve)
{
  (void)(presolve);
  std::cerr << "Scalp: presolve not supported by this backend, ignore this step." << std::endl;
}

void ScaLP::SolverBackend::setThreads(unsigned int t)
{
  (void)(t);
  std::cerr << "Scalp: a thread-limit is not supported by this backend, ignore it." << std::endl;
}

void ScaLP::SolverBackend::setRelativeMIPGap(double d)
{
  (void)(d);
  std::cerr << "Scalp: relative MIP-Gap is not supported by this backend, ignore it." << std::endl;
}

void ScaLP::SolverBackend::setAbsoluteMIPGap(double d)
{
  (void)(d);
  std::cerr << "Scalp: absolute MIP-Gap is not supported by this backend, ignore it." << std::endl;
}
