
#pragma once

#include <map>
#include <vector>

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
      ~SolverSCIP();

      // basic functions
      virtual bool addVariable(ILP::Variable v) override;
      virtual bool addConstraint(ILP::Constraint con) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;

      SCIP *scip;
      std::map<ILP::Variable,SCIP_VAR*> variables;
      std::vector<SCIP_CONS*> constraints;
  };
}
