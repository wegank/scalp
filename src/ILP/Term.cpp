
#include <cmath>
#include <algorithm>

#include <ILP/Term.h>
#include <ILP/Variable.h>
#include <ILP/Exception.h>

namespace ILP
{
  extern double INF();
}

ILP::Term::Term()
{
}

ILP::Term::Term(double con)
{
  constant=con;
}

ILP::Term::Term(ILP::Variable v)
{
  add(v,1);
}

ILP::Term::Term(ILP::Variable v,double coeff)
{
  add(v,coeff);
}

static bool valid(double d)
{
  return d!= ILP::INF() || d!= -ILP::INF() || d!=NAN;
}

void ILP::Term::add(ILP::Variable v,double coeff)
{
  if(valid(coeff))
  {
    auto it = this->sum.find(v);
    if (it!=end(sum))
    {
      it->second+=coeff;
    }
    else
    {
      this->sum.emplace(v,coeff);
    }
  }
  else
  {
    throw ILP::Exception("Only numbers are allowed as coefficients");
  }
}
void ILP::Term::add(double con)
{
  if(valid(con))
  {
    this->constant+=con;
  }
  else
  {
    throw ILP::Exception("Only numbers are allowed as constants");
  }
}

double ILP::Term::getCoefficient(const ILP::Variable& v) const
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

double ILP::Term::setCoefficient(const ILP::Variable& v, double coeff)
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
    throw ILP::Exception("Only numbers are allowed as coefficients");
  }
}

double& ILP::Term::operator[](const Variable& v)
{
  return sum[v];
}

ILP::VariableSet ILP::Term::extractVariables() const
{
  ILP::VariableSet s;
  for(auto &p:sum)
  {
    // extract all non-eliminated variables
    if (p.second!=0) s.insert(p.first);
  }
  return s;
}

bool ILP::Term::isConstant() const
{
  return this->sum.size()==0;
}

std::ostream& operator<<(std::ostream& os, const ILP::Term &t)
{
  for(auto &p:t.sum)
  {
    os << p.first->name << "*" << p.second << " + ";
  }

  //if(t.constant!=0)
  //{
    //if(t.sum.size()!=0) os << "+ ";
    os << t.constant;
  //}
  return os;
}
