
#pragma once

#include <memory>
#include <set>
#include <map>
#include <string>

#include <ScaLP/Exception.h>

namespace ScaLP
{

  // For Users ScaLP::new*Variable() are the only functions of interest.
  
  // The type of the Variables
  enum class VariableType : char
  { BINARY
  , INTEGER
  , REAL
  };

  class VariableBase
  {
    public:
      // TODO: remove (for backward-compatibility)
      using type = ScaLP::VariableType;

      //####################
      // Getter
      //####################
      
      VariableType getType() const;
      double getUpperBound() const;
      double getLowerBound() const;
      const std::string& getName() const;

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
      VariableBase(const std::string& n,double a,double b,type t=type::INTEGER);

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
  Variable newVariable(const std::string& n,double a, double b,VariableType t=VariableType::INTEGER);
  // Specialzations:
  Variable newIntegerVariable(const std::string& n,double a, double b);
  Variable newIntegerVariable(const std::string& n); // free variable
  Variable newRealVariable(const std::string& n,double a, double b);
  Variable newRealVariable(const std::string& n); // free variable
  Variable newBinaryVariable(const std::string& n,double a, double b);
  Variable newBinaryVariable(const std::string& n);


  struct variableComparator{
    bool operator()(const Variable& x,const Variable& y) const
    {
      int c = x->getName().compare(y->getName());
      if(c==0 && x.get()!=y.get())
      { // name-collision
        throw ScaLP::Exception("You defined multiple variables with the name: "+x->getName());
      }
      return c<0;
    }
  };
  using VariableSet = std::set<Variable,variableComparator>;
  using VariableMap = std::map<std::string,Variable>;

  std::ostream& operator<<(std::ostream& oss, const ScaLP::Variable& v);
  std::ostream& operator<<(std::ostream& oss, const ScaLP::VariableBase& v);
  std::ostream& operator<<(std::ostream& oss, const ScaLP::VariableBase* v);

}

