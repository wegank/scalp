#pragma once

#include <ILP/SolverBackend.h>

#include "gurobi_c++.h"

#include <string>
#include <map>

namespace ILP
{
  class SolverGurobi : public SolverBackend
  {
    public:
      SolverGurobi();

      // basic functions
      virtual bool addVariable(const ILP::Variable& v) override;
      virtual bool addVariables(ILP::VariableSet vs) override;
      virtual bool addConstraint(const ILP::Constraint& con) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;
      virtual void setThreads(unsigned int t) override;
      virtual void setRelativeMIPGap(double d) override;
      virtual void setAbsoluteMIPGap(double d) override;

    private:
      // map some values
      char variableType(ILP::VariableType t);
      GRBLinExpr mapTerm(ILP::Term t);
      double mapValue(double d);

      GRBEnv environment;
      GRBModel model;

      std::map<ILP::Variable,GRBVar> variables;
  };
}
