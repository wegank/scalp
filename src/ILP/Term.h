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
      Term(ILP::Variable& v);
      Term(ILP::Variable&& v);

      // an weighted Variable (v*coeff)
      Term(ILP::Variable& v,double coeff);
      Term(ILP::Variable&& v,double coeff);

      // Add a constant value
      void add(double constant);

      // Add v*coeff to the Term.
      // If v is already present the coefficient is increased by coeff
      void add(Variable& v,double coeff);
      void add(Variable&& v,double coeff);

      // get the coefficient of v or zero if not present.
      double getCoefficient(const Variable& v) const;

      // set the coefficient of v to coeff
      // if v is not present it behaves like add.
      void setCoefficient(Variable& v, double coeff);

      double& operator[](const Variable& v);

      ILP::VariableSet extractVariables() const;
      bool isConstant() const;

      // the sum of weighted Variables
      std::map<Variable,double> sum;

      // the constant part
      double constant=0;
  };

}

std::ostream& operator<<(std::ostream& os, const ILP::Term &t);
