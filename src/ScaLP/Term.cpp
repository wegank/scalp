
#include <cmath>
#include <algorithm>

#include <ScaLP/Term.h>
#include <ScaLP/Variable.h>
#include <ScaLP/Exception.h>

namespace ScaLP
{
  extern double INF();
}

ScaLP::Term::Term()
{
}

ScaLP::Term::Term(double con)
{
  constant=con;
}

ScaLP::Term::Term(const ScaLP::Variable& v)
{
  add(v,1);
}
ScaLP::Term::Term(ScaLP::Variable&& v)
{
  add(v,1);
}

ScaLP::Term::Term(const ScaLP::Variable& v,double coeff)
{
  add(v,coeff);
}
ScaLP::Term::Term(ScaLP::Variable&& v,double coeff)
{
  add(v,coeff);
}

static bool valid(double d)
{
  return d!= ScaLP::INF() || d!= -ScaLP::INF() || d!=NAN;
}

void ScaLP::Term::add(const ScaLP::Variable& v,double coeff)
{
  if(coeff==0) return;
  if(valid(coeff))
  {
    auto it = this->sum.find(v);
    if (it!=end(sum))
    {
      it->second+=coeff;
      if(it->second==0) sum.erase(it);
    }
    else
    {
      this->sum.emplace(v,coeff);
    }
  }
  else
  {
    throw ScaLP::Exception("Only numbers are allowed as coefficients");
  }
}
void ScaLP::Term::add(ScaLP::Variable&& v,double coeff)
{
  if(coeff==0) return;
  if(valid(coeff))
  {
    auto it = this->sum.find(v);
    if (it!=end(sum))
    {
      it->second+=coeff;
      if(it->second==0) sum.erase(it);
    }
    else
    {
      this->sum.emplace(v,coeff);
    }
  }
  else
  {
    throw ScaLP::Exception("Only numbers are allowed as coefficients");
  }
}
void ScaLP::Term::add(double con)
{
  if(valid(con))
  {
    this->constant+=con;
  }
  else
  {
    throw ScaLP::Exception("Only numbers are allowed as constants");
  }
}

double ScaLP::Term::getCoefficient(const ScaLP::Variable& v) const
{
  auto it = sum.find(v);
  if(it!=sum.end())
  {
    return it->second;
  }
  else
  {
    return 0;
  }
}

void ScaLP::Term::setCoefficient(ScaLP::Variable& v, double coeff)
{
  if(valid(coeff))
  {
    auto it = sum.find(v);
    if(it!=sum.end())
    {
      it->second=coeff;
    }
    else
    {
      add(v,coeff);
    }
  }
  else
  {
    throw ScaLP::Exception("Only numbers are allowed as coefficients");
  }
}

double& ScaLP::Term::operator[](const Variable& v)
{
  return sum[v];
}

ScaLP::VariableSet ScaLP::Term::extractVariables() const
{
  ScaLP::VariableSet s;
  for(const auto &p:sum)
  {
    // extract all non-eliminated variables
    if (p.second!=0)
    {
      s.emplace(p.first);
    }
  }
  return s;
}

bool ScaLP::Term::isConstant() const
{
  return this->sum.size()==0;
}

bool ScaLP::Term::operator==(const Term &n) const
{
  return this->constant==n.constant and this->sum==n.sum;
}

bool ScaLP::Term::operator!=(const Term &n) const
{
  return not (*this==n);
}

std::ostream& ScaLP::operator<<(std::ostream& os, const ScaLP::Term &t)
{
  for(auto &p:t.sum)
  {
    os << p.first->getName() << "*" << p.second << " + ";
  }

  //if(t.constant!=0)
  //{
    //if(t.sum.size()!=0) os << "+ ";
    os << t.constant;
  //}
  return os;
}
