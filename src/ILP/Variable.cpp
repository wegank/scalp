
#include <memory>
#include <cmath>
#include <ILP/Variable.h>
#include <ILP/Exception.h>

namespace ILP
{
  extern double INF();
}

ILP::Variable ILP::newVariable(std::string n,double a, double b, VariableBase::type t)
{
  Variable v = std::make_shared<VariableBase>(n,a,b,t);
  return v;
};

ILP::Variable ILP::newIntegerVariable(std::string n,double a, double b)
{
  return ILP::newVariable(n,a,b);
}
ILP::Variable ILP::newIntegerVariable(std::string n)
{
  return ILP::newIntegerVariable(n,-ILP::INF(),ILP::INF());
}

ILP::Variable ILP::newRealVariable(std::string n,double a, double b)
{
  return ILP::newVariable(n,a,b,ILP::VariableType::REAL);
}
ILP::Variable ILP::newRealVariable(std::string n)
{
  return ILP::newRealVariable(n,-ILP::INF(),ILP::INF());
}

ILP::Variable ILP::newBinaryVariable(std::string n,double a, double b)
{
  return ILP::newVariable(n,a,b,ILP::VariableType::BINARY);
}
ILP::Variable ILP::newBinaryVariable(std::string n)
{
  return ILP::newVariable(n,false,true,ILP::VariableType::BINARY);
}

using VT = ILP::VariableBase::type;
ILP::VariableBase::VariableBase(std::string n,double a,double b,VT t)
  :name(n),lowerRange(a),upperRange(b),usedType(t)
{
  // Illegal or flipped bounds
  if(a==ILP::INF() || b==-ILP::INF() || a>b)
    throw ILP::Exception("The bounds of "+n+" are illegal: ["+std::to_string(a)+";"+std::to_string(b)+"]");

  // Binary boundaries
  if(t==ILP::VariableBase::type::BINARY)
  {
    if(a+b<0 || a+b>2)
      throw ILP::Exception("The boundaries of the binary variable "+n+" are not right, only 0 and 1 are allowed.");
  }

  // Integer boundaries
  if(t==ILP::VariableBase::type::INTEGER)
  {
    if(a!=-ILP::INF() && fmod(a,1)!=0)
      throw ILP::Exception("Lower bound of Integer-Variable "+n+" is not an Integer("+std::to_string(a)+")");
    if(b!=ILP::INF() && fmod(b,1)!=0)
      throw ILP::Exception("Upper bound of Integer-Variable "+n+" is not an Integer("+std::to_string(b)+")");
  }
}

std::ostream& operator<<(std::ostream& oss, const ILP::Variable& v)
{
  return oss << v->name;
}

std::ostream& operator<<(std::ostream& oss, const ILP::VariableBase& v)
{
  oss << v.name;
  return oss;
}

std::ostream& operator<<(std::ostream& oss, const ILP::VariableBase* v)
{
  oss << v->name;
  return oss;
}
