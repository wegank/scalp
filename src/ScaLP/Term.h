#pragma once

#include <map>

#include <ScaLP/Variable.h>

namespace ScaLP
{

  class Term
  {
    public:
      // an empty term (0)
      Term();

      // Constant term
      Term(double con);

      // implicit v*1
      Term(ScaLP::Variable& v);
      Term(ScaLP::Variable&& v);

      // an weighted Variable (v*coeff)
      Term(ScaLP::Variable& v,double coeff);
      Term(ScaLP::Variable&& v,double coeff);

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

      ScaLP::VariableSet extractVariables() const;
      bool isConstant() const;

      // the sum of weighted Variables
      std::map<Variable,double> sum;

      // the constant part
      double constant=0;

      bool operator==(const Term &n) const;
      bool operator!=(const Term &n) const;
  };
  std::ostream& operator<<(std::ostream& os, const ScaLP::Term &t);

}

