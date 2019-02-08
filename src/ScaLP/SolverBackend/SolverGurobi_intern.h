#pragma once

#include <ScaLP/SolverBackend.h>

#include "gurobi_c++.h"

#include <string>
#include <map>

namespace ScaLP
{
  class SolverGurobi : public SolverBackend
  {
    public:
      SolverGurobi();

      // basic functions
      virtual bool addVariable(const ScaLP::Variable& v) override;
      virtual bool addVariables(const ScaLP::VariableSet& vs) override;
      virtual bool addConstraint(const ScaLP::Constraint& con) override;
      virtual bool setObjective(ScaLP::Objective o) override;
      virtual std::pair<ScaLP::status,ScaLP::Result> solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void setIntFeasTol(double intFeasTol) override;
      virtual void presolve(bool presolve) override;
      virtual void setThreads(unsigned int t) override;
      virtual void setRelativeMIPGap(double d) override;
      virtual void setAbsoluteMIPGap(double d) override;
      virtual void setStartValues(const ScaLP::Result& start) override;

    private:
      // map some values
      char variableType(ScaLP::VariableType t);
      GRBLinExpr mapTerm(ScaLP::Term t);
      double mapValue(double d);

      GRBEnv environment;
      GRBModel model;

      std::map<ScaLP::Variable,GRBVar> variables;
  };
}
