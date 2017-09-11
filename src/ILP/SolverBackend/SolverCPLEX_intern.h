
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
      ~SolverCPLEX()
      {
        try
        {
          env.end();
        }
        catch(IloCplex::Exception& e)
        {
          throw ILP::Exception(e.getMessage());
        }
      };

      // basic functions
      virtual bool addVariable(const ILP::Variable& v) override;
      virtual bool addConstraints(std::list<ILP::Constraint> cons) override;
      virtual bool setObjective(ILP::Objective o) override;
      virtual ILP::status solve() override;
      virtual void reset() override;
      virtual void setConsoleOutput(bool verbose) override;
      virtual void setTimeout(long timeout) override;
      virtual void presolve(bool presolve) override;
      virtual void setThreads(unsigned int t) override;
      virtual void setRelativeMIPGap(double d) override;
      virtual void setAbsoluteMIPGap(double d) override;

      IloEnv env;
      IloModel model;
      std::map<ILP::Variable,IloNumVar> variables;
      bool verbose=true;
      long timeout=0;
      bool presolving=false;
      unsigned int threads=0;

      IloRange createRange(double d, ILP::relation rel,const ILP::Term& t);
      IloRange createRange(const ILP::Term& t, ILP::relation rel,double d);
      IloRange createConstraint3(const ILP::Constraint& c);
      IloExpr mapTerm(const ILP::Term& t);
      IloRange convertConstraint(const ILP::Constraint &c);

      double relMIPGap=-1;
      double absMIPGap=-1;
  };
}
