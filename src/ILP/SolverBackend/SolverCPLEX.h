
#pragma once

#include <ILP/SolverBackend.h>

namespace ILP
{
  class SolverCPLEX : public SolverBackend
  {
    public:
      SolverCPLEX()
      {
      }

      // basic functions
      virtual bool addVariable(ILP::Variable v) override;
      virtual bool addConstraint(ILP::Constraint con) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;

    private:
  };
}
