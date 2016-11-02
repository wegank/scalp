
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
      virtual bool addVariable(ILP::Variable v);
      virtual bool addVariables(ILP::VariableSet vs); // alternative to addVariable
      virtual bool addConstraint(ILP::Constraint con);
      virtual bool addConstraints(std::list<ILP::Constraint> cons); // alternative to addConstraint
      virtual bool setObjective(ILP::Objective o);
      virtual ILP::status solve();
      virtual void reset();
      virtual void setConsoleOutput(bool verbose);

      // experimental: advanced functions
      // You can implement these functions if your ILP-solver can make use of them.
      // You should be able to change an already existing Backend too.
      // TODO: Add more?
      //
      // These functions are called before and after the solve-function is invoked.
      std::function<void()> preSolve;
      std::function<void()> postSolve;

      ILP::Result res;

      virtual ~SolverBackend()
      {
      }

    protected:

      // Protected constructor: No instance allowed
      SolverBackend()
      {
      }

  };

}
