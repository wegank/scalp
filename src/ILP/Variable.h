
#pragma once

#include <memory>
#include <set>
#include <map>
#include <string>

namespace ILP
{

  // For Users ILP::new*Variable() are the only functions of interest.

  class VariableBase
  {
    public:

      // The type of the Variable
      enum class type
      { BINARY
      , INTEGER
      , REAL
      };

      VariableBase(std::string n,double a,double b,type t=type::INTEGER);

      std::string name;
      
      double lowerRange;
      double upperRange;

      type usedType;

    private:
  };

  using Variable = std::shared_ptr<VariableBase>;
  using VariableType = VariableBase::type;

  // Smartconstructors for Variables
  // Generic:
  Variable newVariable(std::string n,double a, double b,VariableBase::type t=VariableType::INTEGER);
  // Specialzations:
  Variable newIntegerVariable(std::string n,double a, double b);
  Variable newIntegerVariable(std::string n); // free variable
  Variable newRealVariable(std::string n,double a, double b);
  Variable newRealVariable(std::string n); // free variable
  Variable newBinaryVariable(std::string n,double a, double b);

  struct variableComparator{
    bool operator()(Variable x,Variable y){
      return x->name<y->name;
    }
  };
  using VariableSet = std::set<Variable,variableComparator>;
  using VariableMap = std::map<std::string,Variable>;


}

std::ostream& operator<<(std::ostream& oss, const ILP::Variable& v);
std::ostream& operator<<(std::ostream& oss, const ILP::VariableBase& v);
std::ostream& operator<<(std::ostream& oss, const ILP::VariableBase* v);
