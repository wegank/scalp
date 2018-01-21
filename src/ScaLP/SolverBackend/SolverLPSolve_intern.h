#pragma once

#include <ScaLP/SolverBackend.h>

#include <lpsolve/lp_lib.h>

#include <string>
#include <map>

namespace ScaLP
{
  class SolverLPSolve : public SolverBackend
  {
    public:
      SolverLPSolve();
      ~SolverLPSolve();

      // basic functions
      virtual bool addVariable(const ScaLP::Variable& v) override;
      virtual bool addConstraint(const ScaLP::Constraint& con) override;
      virtual bool setObjective(ScaLP::Objective o) override;
      virtual std::pair<ScaLP::status,ScaLP::Result> solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;
      virtual void setRelativeMIPGap(double d) override;
      virtual void setAbsoluteMIPGap(double d) override;

    private:
      lprec* lp;
      std::map<ScaLP::Variable,int> variables;
      int variableCounter=0; // index of the last variable
      bool addConstrH(const ScaLP::Term& t, int rel, double rhs, std::string name);
  };
}
