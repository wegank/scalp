
#include <memory>
#include <ILP/Variable.h>

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
