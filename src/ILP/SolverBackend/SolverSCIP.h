
#pragma once

#include <iostream>

#include <ILP/SolverBackend.h>

namespace ILP
{
  class SolverSCIP : public ILP::SolverBackend
  {
    public:
      SolverSCIP()
      {
        preSolve = [](){std::cout << "PresolveTest" << std::endl;};
        postSolve = [](){std::cout << "PostSolveTest" << std::endl;};
      }

      // basic functions
      virtual bool addVariable(ILP::Variable v) override;
      virtual bool addConstraint(ILP::Constraint con) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
  };
}
