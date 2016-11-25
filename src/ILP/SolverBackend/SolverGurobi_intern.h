#pragma once

#include <ILP/SolverBackend.h>

#include "gurobi_c++.h"

namespace ILP
{
  class SolverGurobi : public SolverBackend
  {
    public:
      SolverGurobi();

      // basic functions
      virtual bool addVariable(ILP::Variable v) override;
      virtual bool addVariables(ILP::VariableSet vs) override;
      virtual bool addConstraint(ILP::Constraint con) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;

    private:
      // map some values
      char variableType(ILP::VariableBase::type t);
      GRBLinExpr mapTerm(ILP::Term t);
      double mapValue(double d);

      GRBEnv environment;
      GRBModel model;

      std::map<ILP::Variable,GRBVar> variables;
      bool addConstraint2(ILP::Term lhs, ILP::relation r, ILP::Term rhs);
      bool addConstraint3(ILP::Constraint3 &c3);
  };
}
