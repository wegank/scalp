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

      // set true to disable the Output of the Solver
      bool quiet = false;

      // pass everything to the Solver and run it
      ILP::status solve();

      ILP::Result getResult();

      // set the (new) objective
      void setObjective(Objective o);

      // add a constraint
      void addConstraint(Constraint b);

      // set a new backend
      void setBackend(SolverBackend *b);

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

  };

}

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

ILP::Constraint operator< (ILP::Term tl,ILP::Term tr);
ILP::Constraint operator<=(ILP::Term tl,ILP::Term tr);
ILP::Constraint operator> (ILP::Term tl,ILP::Term tr);
ILP::Constraint operator>=(ILP::Term tl,ILP::Term tr);
ILP::Constraint operator==(ILP::Term tl,ILP::Term tr);

ILP::Constraint operator< (ILP::Constraint tl,ILP::Term tr);
ILP::Constraint operator<=(ILP::Constraint tl,ILP::Term tr);
ILP::Constraint operator> (ILP::Constraint tl,ILP::Term tr);
ILP::Constraint operator>=(ILP::Constraint tl,ILP::Term tr);
ILP::Constraint operator==(ILP::Constraint tl,ILP::Term tr);

ILP::Constraint operator< (ILP::Term tl,ILP::Constraint tr);
ILP::Constraint operator<=(ILP::Term tl,ILP::Constraint tr);
ILP::Constraint operator> (ILP::Term tl,ILP::Constraint tr);
ILP::Constraint operator>=(ILP::Term tl,ILP::Constraint tr);
ILP::Constraint operator==(ILP::Term tl,ILP::Constraint tr);

ILP::Solver &operator<<(ILP::Solver &s,ILP::Objective o);
ILP::Solver &operator<<(ILP::Solver &s,ILP::Constraint o);
