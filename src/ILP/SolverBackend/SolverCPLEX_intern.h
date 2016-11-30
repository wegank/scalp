
#pragma once

#include <map>
#include <list>
#include <vector>

#include <ilcplex/ilocplex.h>

#include <ILP/SolverBackend.h>

namespace ILP
{
  class SolverCPLEX : public ILP::SolverBackend
  {
    public:
      SolverCPLEX();
      ~SolverCPLEX() = default;

      // basic functions
      virtual bool addVariable(ILP::Variable v) override;
      virtual bool addConstraints(std::list<ILP::Constraint> cons) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;

      IloEnv env;
      IloModel model;
      std::map<ILP::Variable,IloNumVar> variables;
      bool verbose=true;
      long timeout=0;
  };
}
