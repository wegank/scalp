#pragma once

#include <ILP/SolverBackend.h>

#include <lpsolve/lp_lib.h>

#include <string>
#include <map>

namespace ILP
{
  class SolverLPSolve : public SolverBackend
  {
    public:
      SolverLPSolve();
      ~SolverLPSolve();

      // basic functions
      virtual bool addVariable(const ILP::Variable& v) override;
      //virtual bool addVariables(ILP::VariableSet vs) override;
      virtual bool addConstraint(const ILP::Constraint& con) override;
      //virtual bool addConstraints(std::list<ILP::Constraint> cons) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;

    private:
      lprec* lp;
      std::map<ILP::Variable,int> variables;
      int variableCounter=0; // index of the last variable
      bool addConstrH(const ILP::Term& t, int rel, double rhs, std::string name);
  };
}
