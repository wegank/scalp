
#pragma once

#include <map>
#include <vector>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/type_paramset.h>

#include <ScaLP/SolverBackend.h>

namespace ScaLP
{
  class SolverSCIP : public ScaLP::SolverBackend
  {
    public:
      SolverSCIP();
      ~SolverSCIP();

      // basic functions
      virtual bool addVariable(const ScaLP::Variable& v) override;
      virtual bool addConstraint(const ScaLP::Constraint& con) override;
      virtual bool setObjective(ScaLP::Objective o) override;
      virtual ScaLP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;
      virtual void setThreads(unsigned int t) override;

      SCIP *scip;
      std::map<ScaLP::Variable,SCIP_VAR*> variables;
      std::vector<SCIP_CONS*> constraints;
  };
}
