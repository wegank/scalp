
#pragma once

#include <memory>
#include <set>
#include <map>
#include <string>

#include <ILP/Exception.h>

namespace ILP
{

  // For Users ILP::new*Variable() are the only functions of interest.
  
  // The type of the Variables
  enum class VariableType
  { BINARY
  , INTEGER
  , REAL
  };

  class VariableBase
  {
    public:
      // TODO: remove (for backward-compatibility)
      using type = ILP::VariableType;

      //####################
      // Getter
      //####################
      
      VariableType getType() const;
      double getUpperBound() const;
      double getLowerBound() const;
      std::string getName() const;

      //####################
      // Setter
      //####################

      void unsafeSetType(VariableType t);
      void unsafeSetUpperBound(double d);
      void unsafeSetLowerBound(double d);
      void unsafeSetName(std::string s);

      //####################
      // Construction (use the smartconstructors below)
      //####################
      VariableBase(std::string n,double a,double b,type t=type::INTEGER);

    private:
      type usedType;
      std::string name;
      double lowerRange;
      double upperRange;
  };

  using Variable = std::shared_ptr<VariableBase>;

  //####################
  // Smartconstructors
  //####################
  // Generic:
  Variable newVariable(std::string n,double a, double b,VariableType t=VariableType::INTEGER);
  // Specialzations:
  Variable newIntegerVariable(std::string n,double a, double b);
  Variable newIntegerVariable(std::string n); // free variable
  Variable newRealVariable(std::string n,double a, double b);
  Variable newRealVariable(std::string n); // free variable
  Variable newBinaryVariable(std::string n,double a, double b);
  Variable newBinaryVariable(std::string n);


  struct variableComparator{
    bool operator()(Variable x,Variable y){
      if(x->getName()==y->getName() && x.get()!=y.get())
      { // name-collision
        throw ILP::Exception("You defined multiple variables with the name: "+x->getName());
      }
      return x->getName()<y->getName();
    }
  };
  using VariableSet = std::set<Variable,variableComparator>;
  using VariableMap = std::map<std::string,Variable>;

  std::ostream& operator<<(std::ostream& oss, const ILP::Variable& v);
  std::ostream& operator<<(std::ostream& oss, const ILP::VariableBase& v);
  std::ostream& operator<<(std::ostream& oss, const ILP::VariableBase* v);

}

