
#pragma once

#include <iostream>

#include <scip/scip.h>
//#include <objscip/objscip.h>
#include <scip/scipdefplugins.h>

#include <ILP/SolverBackend.h>

namespace ILP
{
  class SolverSCIP : public ILP::SolverBackend
  {
    public:
      SolverSCIP();

      // basic functions
      //virtual bool addVariable(ILP::Variable v) override;
      virtual bool addVariables(ILP::VariableSet vs); // alternative to addVariable
      virtual bool addConstraint(ILP::Constraint con) override;
      //virtual bool addConstraints(std::list<ILP::Constraint> cons); // alternative to addConstraint
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;

      SCIP *scip;
      std::map<ILP::Variable,SCIP_VAR*> variables;
  };
}
