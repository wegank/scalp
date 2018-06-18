
#pragma once

#include <functional>
#include <vector>

#include <ScaLP/Variable.h>
#include <ScaLP/Constraint.h>
#include <ScaLP/Objective.h>
#include <ScaLP/Result.h>

namespace ScaLP
{

  enum class Feature: char
  { LP
  , ILP
  , QP
  , MILP
  , INDICATOR_CONSTRAINTS
  , LOGICAL_OPERATORS
  , WARMSTART
  };

  class Features
  {
    public:
    bool lp=false;
    bool ilp=false;
    bool qp=false;
    bool milp=false;
    bool indicators=false;
    bool logical=false;
    bool warmstart=false;
  };

  class SolverBackend
  {
    public:

      //####################
      // basic functions
      //####################
      virtual bool addVariable(const ScaLP::Variable& v);
      virtual bool addVariables(const ScaLP::VariableSet& vs); // alternative to addVariable
      virtual bool addConstraint(const ScaLP::Constraint& con);
      virtual bool addConstraints(const std::vector<ScaLP::Constraint>& cons); // alternative to addConstraint
      virtual bool setObjective(ScaLP::Objective o);
      virtual std::pair<ScaLP::status,ScaLP::Result> solve();
      virtual void reset();
      virtual void setConsoleOutput(bool verbose);
      virtual void setTimeout(long timeout);
      virtual void setIntFeasTol(double intFeasTol);
      virtual void presolve(bool presolve);
      virtual void setThreads(unsigned int t);
      virtual void setRelativeMIPGap(double d);
      virtual void setAbsoluteMIPGap(double d);
      virtual void setStartValues(const ScaLP::Result& start);

      Features features;
      bool featureSupported(ScaLP::Feature f) const;

      double objectiveOffset=0;

      virtual ~SolverBackend()
      {
      }

      // The name of the backend-solver
      std::string name;

    protected:

      // Protected constructor: No instance allowed
      SolverBackend()
      {
      }

  };

}
