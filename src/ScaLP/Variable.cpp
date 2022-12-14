
#include <memory>
#include <cmath>
#include <ScaLP/Variable.h>
#include <ScaLP/Exception.h>

namespace ScaLP
{
  extern double INF();
}

ScaLP::Variable ScaLP::newVariable(const std::string& n,double a, double b, double s, VariableType t)
{
  Variable v = std::make_shared<VariableBase>(n,a,b,s,t);
  return v;
};
ScaLP::Variable ScaLP::newVariable(const std::string& n,double a, double b, VariableType t)
{
  Variable v = std::make_shared<VariableBase>(n,a,b,INFINITY,t);
  return v;
};

ScaLP::Variable ScaLP::newIntegerVariable(const std::string& n,double a, double b, int start)
{
  return ScaLP::newVariable(n,a,b,start);
}
ScaLP::Variable ScaLP::newIntegerVariable(const std::string& n,double a, double b)
{
  return ScaLP::newVariable(n,a,b);
}
ScaLP::Variable ScaLP::newIntegerVariable(const std::string& n)
{
  return ScaLP::newIntegerVariable(n,-ScaLP::INF(),ScaLP::INF());
}

ScaLP::Variable ScaLP::newRealVariable(const std::string& n,double a, double b, double start)
{
  return ScaLP::newVariable(n,a,b,start,ScaLP::VariableType::REAL);
}
ScaLP::Variable ScaLP::newRealVariable(const std::string& n,double a, double b)
{
  return ScaLP::newVariable(n,a,b,ScaLP::VariableType::REAL);
}
ScaLP::Variable ScaLP::newRealVariable(const std::string& n)
{
  return ScaLP::newRealVariable(n,-ScaLP::INF(),ScaLP::INF());
}

ScaLP::Variable ScaLP::newBinaryVariable(const std::string& n,double a, double b, bool start)
{
  return ScaLP::newVariable(n,a,b,start,ScaLP::VariableType::BINARY);
}
ScaLP::Variable ScaLP::newBinaryVariable(const std::string& n,double a, double b)
{
  return ScaLP::newVariable(n,a,b,ScaLP::VariableType::BINARY);
}
ScaLP::Variable ScaLP::newBinaryVariable(const std::string& n)
{
  return ScaLP::newVariable(n,false,true,ScaLP::VariableType::BINARY);
}

ScaLP::VariableBase::VariableBase(const std::string& n,double a,double b,double s,ScaLP::VariableType t)
  : usedType(t), name(n), lowerRange(a), upperRange(b), start(s)
{
  // Illegal or flipped bounds
  if(a==ScaLP::INF() || b==-ScaLP::INF() || a>b)
    throw ScaLP::Exception("The bounds of "+n+" are illegal: ["+std::to_string(a)+";"+std::to_string(b)+"]");

  // Binary boundaries
  if(t==ScaLP::VariableType::BINARY)
  {
    if(a+b<0 || a+b>2)
      throw ScaLP::Exception("The boundaries of the binary variable "+n+" are not right, only 0 and 1 are allowed.");
  }

  // Integer boundaries
  if(t==ScaLP::VariableType::INTEGER)
  {
    if(a!=-ScaLP::INF() && fmod(a,1)!=0)
      throw ScaLP::Exception("Lower bound of Integer-Variable "+n+" is not an Integer("+std::to_string(a)+")");
    if(b!=ScaLP::INF() && fmod(b,1)!=0)
      throw ScaLP::Exception("Upper bound of Integer-Variable "+n+" is not an Integer("+std::to_string(b)+")");
  }
}

std::ostream& ScaLP::operator<<(std::ostream& oss, const ScaLP::Variable& v)
{
  return oss << v->getName();
}

std::ostream& ScaLP::operator<<(std::ostream& oss, const ScaLP::VariableBase& v)
{
  oss << v.getName();
  return oss;
}

std::ostream& ScaLP::operator<<(std::ostream& oss, const ScaLP::VariableBase* v)
{
  oss << v->getName();
  return oss;
}



ScaLP::VariableType ScaLP::VariableBase::getType() const
{
  return usedType;
}
void ScaLP::VariableBase::unsafeSetType(ScaLP::VariableType t)
{
  this->usedType = t;
}
double ScaLP::VariableBase::getUpperBound() const
{
  return upperRange;
}
void ScaLP::VariableBase::unsafeSetUpperBound(double d)
{
  this->upperRange = d;
}
double ScaLP::VariableBase::getLowerBound() const
{
  return lowerRange;
}
void ScaLP::VariableBase::unsafeSetLowerBound(double d)
{
  this->lowerRange = d;
}
const std::string& ScaLP::VariableBase::getName() const
{
  return this->name;
}
double ScaLP::VariableBase::getStart() const
{
  return this->start;
}
void ScaLP::VariableBase::unsafeSetName(std::string s)
{
  this->name=s;
}
