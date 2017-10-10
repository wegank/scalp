#pragma once

#include <list>
#include <initializer_list>
#include <string>

#include <ScaLP/Constraint.h>
#include <ScaLP/Objective.h>
#include <ScaLP/Objective.h>
#include <ScaLP/Result.h>
#include <ScaLP/SolverBackend.h>
#include <ScaLP/Term.h>
#include <ScaLP/Variable.h>

namespace ScaLP
{

  // a representation of infinity (used for variable-ranges only)
  double INF();

  class Solver
  {
    public:

      // The memory of the Pointer is managed by the Solver
      Solver(ScaLP::SolverBackend *b);
      Solver(std::list<std::string> ls);
      Solver(std::list<ScaLP::Feature> fs, std::list<std::string> ls);
      Solver(std::initializer_list<std::string> ls);

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
      bool presolve = true;

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
      ScaLP::status solve();

      ScaLP::Result getResult();


      //####################
      // Backend
      //####################

      // set a new backend (The old one is removed automatically)
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

      // check if the used solver supports the given feature
      bool featureSupported(ScaLP::Feature f);

      // write the LP-Format-representation in a file
      void writeLP(std::string file) const;

      ~Solver();

    private:
      // The used objective
      Objective objective;

      // The used constraints
      std::list<Constraint> cons;

      // The backend used to solve the objective
      ScaLP::SolverBackend *back;

      ScaLP::Result result;

      // extract the Variables from the Constraints and the Objective
      // to avoid unused variables.
      ScaLP::VariableSet extractVariables(std::list<Constraint> c,Objective o) const;

      double absMIPGap=-1;
      double relMIPGap=-1;

  };

  ScaLP::Term operator*(ScaLP::Variable v,double coeff);
  ScaLP::Term operator*(double coeff, ScaLP::Variable v);
  ScaLP::Term operator*(double coeff, ScaLP::Term t);
  ScaLP::Term operator*(ScaLP::Term t , double coeff);

  ScaLP::Term operator+(ScaLP::Term tl, ScaLP::Term tr);
  ScaLP::Term operator-(ScaLP::Term tl, ScaLP::Term tr);

  ScaLP::Term& operator+=(ScaLP::Term& tl, ScaLP::Term tr);
  ScaLP::Term& operator-=(ScaLP::Term& tl, ScaLP::Term tr);
  ScaLP::Term& operator*=(ScaLP::Term& tl, double d);

  ScaLP::Term operator-(ScaLP::Variable v);
  ScaLP::Term operator-(ScaLP::Term t);

  ScaLP::Constraint operator<=(ScaLP::Variable tl,double tr);
  ScaLP::Constraint operator>=(ScaLP::Variable tl,double tr);
  ScaLP::Constraint operator==(ScaLP::Variable tl,double tr);
  ScaLP::Constraint operator<=(double tl,ScaLP::Variable tr);
  ScaLP::Constraint operator>=(double tl,ScaLP::Variable tr);
  ScaLP::Constraint operator==(double tl,ScaLP::Variable tr);

  ScaLP::Constraint operator<=(ScaLP::Term tl,double tr);
  ScaLP::Constraint operator>=(ScaLP::Term tl,double tr);
  ScaLP::Constraint operator==(ScaLP::Term tl,double tr);
  ScaLP::Constraint operator<=(double tl,ScaLP::Term tr);
  ScaLP::Constraint operator>=(double tl,ScaLP::Term tr);
  ScaLP::Constraint operator==(double tl,ScaLP::Term tr);

  ScaLP::Constraint operator<=(ScaLP::Constraint tl,double tr);
  ScaLP::Constraint operator>=(ScaLP::Constraint tl,double tr);
  ScaLP::Constraint operator==(ScaLP::Constraint tl,double tr);

  ScaLP::Constraint operator<=(double tl,ScaLP::Constraint tr);
  ScaLP::Constraint operator>=(double tl,ScaLP::Constraint tr);
  ScaLP::Constraint operator==(double tl,ScaLP::Constraint tr);

  ScaLP::Solver &operator<<(ScaLP::Solver &s,ScaLP::Objective o);
  ScaLP::Solver &operator<<(ScaLP::Solver &s,ScaLP::Constraint& o);
  ScaLP::Solver &operator<<(ScaLP::Solver &s,ScaLP::Constraint&& o);

  ScaLP::Constraint operator>>=(ScaLP::Constraint i,ScaLP::Constraint c);
  #define then >>=

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
