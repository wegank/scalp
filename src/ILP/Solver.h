#pragma once

#include <list>
#include <string>

#include <ILP/Constraint.h>
#include <ILP/Objective.h>
#include <ILP/Objective.h>
#include <ILP/Result.h>
#include <ILP/SolverBackend.h>
#include <ILP/Term.h>
#include <ILP/Variable.h>

namespace ILP
{

  // a representation of Infinity
  double INF();

  class Solver
  {
    public:

      // The memory of the Pointer is managed by the Solver
      Solver(ILP::SolverBackend *b);

      //####################
      // general Parameters
      //####################

      // set true to disable the Output of the Solver
      bool quiet = false;

      // timeout for the solving progress
      // it is measured in seconds, zero is no limit.
      // you can use suffixes like _hours, _seconds (_hour, etc. works too)
      // e. g. timeout = 2_hours + 15_minutes + 30_seconds
      long timeout = 0;

      // use presolvers to simplify the model
      bool presolve = false;

      // enable debug-output of scalp and possibly a file with the LP-output of the backend.
      // TODO: not implemented yet.
      //bool debug = false;

      // Threads used by the solver (0 means auto-detection)
      int threads = 0;


      //####################
      // Problem-Construction
      //####################

      // set the (new) objective
      void setObjective(Objective o);

      // add a constraint
      void addConstraint(Constraint& b);
      void addConstraint(Constraint&& b);


      //####################
      // Solving
      //####################

      // pass everything to the Solver and run it
      ILP::status solve();

      ILP::Result getResult();


      //####################
      // Backend
      //####################

      // set a new backend
      void setBackend(SolverBackend *b);

      std::string getBackendName() const;

      // reset the Solver (removes all Constraints, etc)
      void reset();


      //####################
      // MIP-Parameters
      //####################

      // set the relative MIP-Gap to d (d>=0)
      bool setRelativeMIPGap(double d);

      // set the absolute MIP-Gap to d (d>=0)
      bool setAbsoluteMIPGap(double d);

      // resets the MIP-Gap to the solvers default
      void resetMIPGap();


      //####################
      // miscellaneous
      //####################

      // return the LP-Format-representation as a string
      std::string showLP() const;

      // write the LP-Format-representation in a file
      void writeLP(std::string file) const;

      ~Solver();

    private:
      // The used objective
      Objective objective;

      // The used constraints
      std::list<Constraint> cons;

      // The backend used to solve the objective
      ILP::SolverBackend *back;

      ILP::Result result;

      // extract the Variables from the Constraints and the Objective
      // to avoid unused variables.
      ILP::VariableSet extractVariables(std::list<Constraint> c,Objective o) const;

      double absMIPGap=-1;
      double relMIPGap=-1;

  };

  ILP::Term operator*(ILP::Variable v,double coeff);
  ILP::Term operator*(double coeff, ILP::Variable v);
  ILP::Term operator*(double coeff, ILP::Term t);
  ILP::Term operator*(ILP::Term t , double coeff);

  ILP::Term operator+(ILP::Term tl, ILP::Term tr);
  ILP::Term operator-(ILP::Term tl, ILP::Term tr);

  ILP::Term& operator+=(ILP::Term& tl, ILP::Term tr);
  ILP::Term& operator-=(ILP::Term& tl, ILP::Term tr);
  ILP::Term& operator*=(ILP::Term& tl, double d);

  ILP::Term operator-(ILP::Variable v);
  ILP::Term operator-(ILP::Term t);

  ILP::Constraint operator<=(ILP::Term tl,double tr);
  ILP::Constraint operator>=(ILP::Term tl,double tr);
  ILP::Constraint operator==(ILP::Term tl,double tr);
  ILP::Constraint operator<=(double tl,ILP::Term tr);
  ILP::Constraint operator>=(double tl,ILP::Term tr);
  ILP::Constraint operator==(double tl,ILP::Term tr);

  ILP::Constraint operator<=(ILP::Constraint tl,double tr);
  ILP::Constraint operator>=(ILP::Constraint tl,double tr);
  ILP::Constraint operator==(ILP::Constraint tl,double tr);

  ILP::Constraint operator<=(double tl,ILP::Constraint tr);
  ILP::Constraint operator>=(double tl,ILP::Constraint tr);
  ILP::Constraint operator==(double tl,ILP::Constraint tr);

  ILP::Solver &operator<<(ILP::Solver &s,ILP::Objective o);
  ILP::Solver &operator<<(ILP::Solver &s,ILP::Constraint& o);
  ILP::Solver &operator<<(ILP::Solver &s,ILP::Constraint&& o);

}


// time suffixes
// visual studio < 2015 does not support constexpr and user-defined-literals
#if(not defined(_MSC_VER) ||  _MSC_VER >= 1900)

#define SUFFIX(A,MULT) \
constexpr long double operator"" A (long double n) \
{ \
  return n*MULT; \
} \
constexpr unsigned long long int operator"" A (unsigned long long int n) \
{ \
  return n*MULT; \
}

SUFFIX(_days    , 60*60*24)
SUFFIX(_day     , 60*60*24)
SUFFIX(_hours   , 60*60)
SUFFIX(_hour    , 60*60)
SUFFIX(_minutes , 60)
SUFFIX(_minute  , 60)
SUFFIX(_seconds , 1)
SUFFIX(_second  , 1)

#endif // _MSC_VER

