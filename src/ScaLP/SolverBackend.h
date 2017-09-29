
#pragma once

#include <functional>
#include <list>

#include <ScaLP/Variable.h>
#include <ScaLP/Constraint.h>
#include <ScaLP/Objective.h>
#include <ScaLP/Result.h>

namespace ScaLP
{

  class SolverBackend
  {
    public:

      //####################
      // basic functions
      //####################
      virtual bool addVariable(const ScaLP::Variable& v);
      virtual bool addVariables(ScaLP::VariableSet vs); // alternative to addVariable
      virtual bool addConstraint(const ScaLP::Constraint& con);
      virtual bool addConstraints(std::list<ScaLP::Constraint> cons); // alternative to addConstraint
      virtual bool setObjective(ScaLP::Objective o);
      virtual ScaLP::status solve();
      virtual void reset();
      virtual void setConsoleOutput(bool verbose);
      virtual void setTimeout(long timeout);
      virtual void presolve(bool presolve);
      virtual void setThreads(unsigned int t);
      virtual void setRelativeMIPGap(double d);
      virtual void setAbsoluteMIPGap(double d);


      ScaLP::Result res;

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
