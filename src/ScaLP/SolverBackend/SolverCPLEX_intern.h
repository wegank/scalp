
#pragma once

#include <map>
#include <vector>

#include <ilcplex/ilocplex.h>

#include <ScaLP/SolverBackend.h>

namespace ScaLP
{
  class SolverCPLEX : public ScaLP::SolverBackend
  {
    public:
      SolverCPLEX();
      ~SolverCPLEX()
      {
        try
        {
          env.end();
          if(startVar!=nullptr)
          {
            startVar->end();
            delete startVar;
          }
          if(startVal!=nullptr)
          {
            startVal->end();
            delete startVal;
          }
        }
        catch(IloCplex::Exception& e)
        {
          throw ScaLP::Exception(e.getMessage());
        }
      };

      // basic functions
      virtual bool addVariable(const ScaLP::Variable& v) override;
      virtual bool addConstraints(const std::vector<ScaLP::Constraint>& cons) override;
      virtual bool setObjective(ScaLP::Objective o) override;
      virtual std::pair<ScaLP::status,ScaLP::Result> solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;
      virtual void setThreads(unsigned int t) override;
      virtual void setRelativeMIPGap(double d) override;
      virtual void setAbsoluteMIPGap(double d) override;
      virtual void setStartValues(const ScaLP::Result& start);

      IloEnv env;
      IloModel model;
      std::map<ScaLP::Variable,IloNumVar> variables;
      bool verbose=true;
      long timeout=0;
      bool presolving=false;
      unsigned int threads=0;

      IloConstraint createRange(double d, ScaLP::relation rel,const ScaLP::Term& t);
      IloConstraint createRange(const ScaLP::Term& t, ScaLP::relation rel,double d);
      IloConstraint createConstraint3(const ScaLP::Constraint& c);
      IloExpr mapTerm(const ScaLP::Term& t);
      IloConstraint convertConstraint(const ScaLP::Constraint &c);

      double relMIPGap=-1;
      double absMIPGap=-1;

      IloNumVarArray* startVar=nullptr;
      IloNumArray* startVal=nullptr;
  };
}
