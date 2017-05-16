
#pragma once

#include <map>
#include <vector>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/type_paramset.h>

#include <ILP/SolverBackend.h>

namespace ILP
{
  class SolverSCIP : public ILP::SolverBackend
  {
    public:
      SolverSCIP();
      ~SolverSCIP();

      // basic functions
      virtual bool addVariable(const ILP::Variable& v) override;
      virtual bool addConstraint(const ILP::Constraint& con) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;
      virtual void setThreads(unsigned int t) override;

      SCIP *scip;
      std::map<ILP::Variable,SCIP_VAR*> variables;
      std::vector<SCIP_CONS*> constraints;
  };
}
