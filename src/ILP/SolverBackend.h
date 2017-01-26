
#pragma once

#include <functional>
#include <list>

#include <ILP/Variable.h>
#include <ILP/Constraint.h>
#include <ILP/Objective.h>
#include <ILP/Result.h>

namespace ILP
{

  class SolverBackend
  {
    public:

      // basic functions
      virtual bool addVariable(const ILP::Variable& v);
      virtual bool addVariables(ILP::VariableSet vs); // alternative to addVariable
      virtual bool addConstraint(const ILP::Constraint& con);
      virtual bool addConstraints(std::list<ILP::Constraint> cons); // alternative to addConstraint
      virtual bool setObjective(ILP::Objective o);
      virtual ILP::status solve();
      virtual void reset();
      virtual void setConsoleOutput(bool verbose);
      virtual void setTimeout(long timeout);
      virtual void presolve(bool presolve);

      ILP::Result res;

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
