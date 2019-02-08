#include <ScaLP/SolverBackend.h>

#include <iostream>

#include <ScaLP/Exception.h>

bool ScaLP::SolverBackend::addVariable(const ScaLP::Variable& v)
{
  (void)(v);
  return false;
}

bool ScaLP::SolverBackend::addVariables(const ScaLP::VariableSet& vs)
{
  for(const ScaLP::Variable& v:vs)
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

bool ScaLP::SolverBackend::addConstraints(const std::vector<ScaLP::Constraint>& cons)
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

std::pair<ScaLP::status,ScaLP::Result> ScaLP::SolverBackend::solve()
{
  return {ScaLP::status::ERROR,{}};
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

void ScaLP::SolverBackend::setIntFeasTol(double intFeasTol)
{
  (void)(intFeasTol);
  throw ScaLP::Exception("Scalp: You need to implement the setIntFeasTol function in the backend.");
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
void ScaLP::SolverBackend::setStartValues(const ScaLP::Result& start)
{
  (void)(start);
  std::cerr << "Scalp: Warm-start not supported by this backend, ignore it." << std::endl;
}

bool ScaLP::SolverBackend::featureSupported(ScaLP::Feature f) const
{
  switch(f)
  {
    case ScaLP::Feature::LP : return this->features.lp;
    case ScaLP::Feature::ILP : return this->features.ilp;
    case ScaLP::Feature::QP : return this->features.qp;
    case ScaLP::Feature::MILP : return this->features.milp;
    case ScaLP::Feature::INDICATOR_CONSTRAINTS : return this->features.indicators;
    case ScaLP::Feature::LOGICAL_OPERATORS : return this->features.logical;
    case ScaLP::Feature::WARMSTART : return this->features.warmstart;
  }
  return false;
}
