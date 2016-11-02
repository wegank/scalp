#pragma once

#include <map>

#include <ILP/Variable.h>

namespace ILP
{

  class Term
  {
    public:
      // an empty term (0)
      Term();

      // Constant term
      Term(double con);

      // implicit v*1
      Term(ILP::Variable v);

      // an weighted Variable (v*coeff)
      Term(ILP::Variable v,double coeff);

      // the sum of weighted Variables
      std::map<Variable,double> sum;

      // the constant part
      double constant=0;

      void add(Variable v,double coeff);
      void add(double constant);

      ILP::VariableSet extractVariables();
      bool isConstant();
  };

}

std::ostream& operator<<(std::ostream& os, const ILP::Term &t);
