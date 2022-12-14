
#include <ScaLP/Constraint.h>
#include <ScaLP/Exception.h>

namespace ScaLP
{
  extern double INF();
}

ScaLP::Constraint::~Constraint()
{
}

void ScaLP::Constraint::setName(const std::string& n)
{
  name=n;
}

static bool isIndicator(const ScaLP::Constraint& c)
{
  return c.ctype==ScaLP::Constraint::type::CEQ // ... == ...
    and c.lbound==c.ubound and (c.lbound == 0 or c.lbound == 1) // ... == 1 or 0
    and (c.term.sum.size()==1 and c.term.constant==0) // x == [1,0]
    and c.term.sum.begin()->first->getType()==ScaLP::VariableType::BINARY
    and c.term.sum.begin()->second==1; // 1*x(binary) == 1 or 0
}

ScaLP::Constraint::Constraint(ScaLP::Constraint i, ScaLP::Constraint c)
: Constraint(c)
{
  if(isIndicator(i))
  {
    this->indicator = std::make_shared<Constraint>(i);
  }
  else
  {
    throw ScaLP::Exception("The used constraint is not an indicator. (binary variable == 0 or 1)");
  }
}


std::string ScaLP::Constraint::showRelation(relation r)
{
  switch(r)
  {
    case ScaLP::relation::LESS_EQ_THAN: return "<=";
    case ScaLP::relation::MORE_EQ_THAN: return ">=";
    case ScaLP::relation::EQUAL: return "=";
    default:return "";
  }
  return "";
}

ScaLP::VariableSet ScaLP::Constraint::extractVariables() const
{
  auto s = term.extractVariables();
  if(this->indicator!=nullptr)
  {
    auto vi = this->indicator->term.extractVariables();
    s.insert(vi.begin(),vi.end());
  }
  return s;
}

std::string ScaLP::Constraint::show() const
{
  //TODO:
  throw ScaLP::Exception("This Constraint-type is not implemented yet.");
  return "";
}

static bool relationsNotCompatible(ScaLP::relation l, ScaLP::relation r)
{
  using rel = ScaLP::relation;
  return (l==rel::EQUAL or r==rel::EQUAL) // a == b == c, a <= b == c (does not make much sense?)
    or (l==rel::MORE_EQ_THAN and r==rel::LESS_EQ_THAN) // a >= b <= c (two upper bounds)
    or (l==rel::LESS_EQ_THAN and r==rel::MORE_EQ_THAN) // a <= b >= c (two lower bounds)
    ;
}

static inline void checkRelationCompatibility(ScaLP::relation l, ScaLP::relation r)
{
  if(relationsNotCompatible(l,r))
  {
    throw ScaLP::Exception("The used relations ("+ScaLP::Constraint::showRelation(l)+") and ("+ScaLP::Constraint::showRelation(r)+") are not compatible");
  }
}

// basic constructors
ScaLP::Constraint::Constraint(double l, relation rel, const ScaLP::Term& r)
  : lbound(l), lrel(rel), rrel(rel), ctype(ScaLP::Constraint::type::C2L), term(r)
{
  if(rel==ScaLP::relation::EQUAL)
  {
    ctype=ScaLP::Constraint::type::CEQ;
    rrel=ScaLP::relation::EQUAL;
    ubound=lbound;
  }
  else if(rel==ScaLP::relation::MORE_EQ_THAN)
  {
    ubound = -ScaLP::INF();
  }
}
ScaLP::Constraint::Constraint(const ScaLP::Term& l, relation rel, double r)
  : ubound(r), lrel(rel), rrel(rel), ctype(ScaLP::Constraint::type::C2R), term(l)
{
  if(rel==ScaLP::relation::EQUAL)
  {
    ctype=ScaLP::Constraint::type::CEQ;
    lrel=ScaLP::relation::EQUAL;
    lbound=ubound;
  }
  else if(rel==ScaLP::relation::MORE_EQ_THAN)
  {
    lbound = ScaLP::INF();
  }
}
ScaLP::Constraint::Constraint(double lb, relation lrel, const ScaLP::Term& t, relation rrel,double ub)
  : lbound(lb), ubound(ub), lrel(lrel), rrel(rrel), ctype(ScaLP::Constraint::type::C3), term(t)
{
  checkRelationCompatibility(lrel,rrel);
}
ScaLP::Constraint::Constraint(double l, relation rel, ScaLP::Term&& r)
  : lbound(l), lrel(rel), rrel(rel), ctype(ScaLP::Constraint::type::C2L), term(r)
{
  if(rel==ScaLP::relation::EQUAL)
  {
    ctype=ScaLP::Constraint::type::CEQ;
    rrel=ScaLP::relation::EQUAL;
    ubound=lbound;
  }
  else if(rel==ScaLP::relation::MORE_EQ_THAN)
  {
    ubound = -ScaLP::INF();
  }
}
ScaLP::Constraint::Constraint(ScaLP::Term&& l, relation rel, double r)
  : ubound(r), lrel(rel), rrel(rel), ctype(ScaLP::Constraint::type::C2R), term(l)
{
  if(rel==ScaLP::relation::EQUAL)
  {
    ctype=ScaLP::Constraint::type::CEQ;
    lrel=ScaLP::relation::EQUAL;
    lbound=ubound;
  }
  else if(rel==ScaLP::relation::MORE_EQ_THAN)
  {
    lbound = ScaLP::INF();
  }
}
ScaLP::Constraint::Constraint(double lb, relation lrel, ScaLP::Term&& t, relation rrel,double ub)
  : lbound(lb), ubound(ub), lrel(lrel), rrel(rrel), ctype(ScaLP::Constraint::type::C3), term(t)
{
  checkRelationCompatibility(lrel,rrel);
}

// combination constructors
ScaLP::Constraint::Constraint(const ScaLP::Constraint& lhs, relation rel, double ub)
  : Constraint(lhs.lbound,lhs.lrel,lhs.term,rel,ub)
{
}
ScaLP::Constraint::Constraint(double lb, relation rel, const ScaLP::Constraint& rhs)
  : Constraint(lb,rel,rhs.term,rhs.rrel,rhs.ubound)
{
}

// named constraint constructors
ScaLP::Constraint::Constraint(const std::string& n, const ScaLP::Constraint& c)
  : ScaLP::Constraint(c)
{
  name=n;
}
ScaLP::Constraint::Constraint(const std::string& n, ScaLP::Constraint&& c)
  : ScaLP::Constraint(c)
{
  name=n;
}
ScaLP::Constraint::Constraint(const std::pair<std::string,ScaLP::Constraint>& p)
  : ScaLP::Constraint(p.second)
{
  name=p.first;
}

static bool between(double l, double x, double r)
{
  return l<=x and x<=r;
}

static double reduceTerm(const ScaLP::Constraint& c, const ScaLP::Result& sol)
{
  double res=0;
  for(auto&p:c.term.sum)
  {
    auto it = sol.values.find(p.first);
    if(it!=sol.values.end())
    {
      if(not between(p.first->getLowerBound(), it->second, p.first->getUpperBound()))
      {

        throw ScaLP::Exception("The warm-start value"+std::to_string(it->second)+" of variable "+p.first->getName()+" is not in its domain.");
      }
      else
      {
        res+=p.second*(it->second);
      }
    }
    else
    {
      throw ScaLP::Exception("There are variables in this Constraint, that aren't covered by the result.");
    }
  }
  return res;
}

static bool relationTrue(double a, ScaLP::relation r, double b)
{
  switch(r)
  {
    case ScaLP::relation::LESS_EQ_THAN: return a<=b;
    case ScaLP::relation::MORE_EQ_THAN: return a>=b;
    case ScaLP::relation::EQUAL: return a==b;
    default: return false;
  }
}

bool ScaLP::Constraint::isFeasible(const ScaLP::Result& sol)
{
  if(indicator)
  {
    auto& p= *indicator->term.sum.begin();
    if(not relationTrue(sol.values.at(p.first)*p.second,ScaLP::relation::EQUAL,indicator->lbound)) return false;
  }

  double t = reduceTerm(*this,sol);
  switch(ctype)
  {
    case ScaLP::Constraint::type::C2L:
      return relationTrue(lbound,lrel,t);
    case ScaLP::Constraint::type::C2R:
      return relationTrue(t,rrel,ubound);
    case ScaLP::Constraint::type::CEQ:
      return relationTrue(lbound,ScaLP::relation::EQUAL,t);
    case ScaLP::Constraint::type::C3:
      return relationTrue(lbound,lrel,t) and relationTrue(t,rrel,ubound);
  }
}

std::ostream& ScaLP::operator<<(std::ostream& os, const ScaLP::Constraint &c)
{
  switch(c.ctype)
  {
    case ScaLP::Constraint::type::C2L:
      os << c.lbound << " " << ScaLP::Constraint::showRelation(c.lrel) << " " << c.term;
      break;
    case ScaLP::Constraint::type::C2R:
      os << c.term << " " << ScaLP::Constraint::showRelation(c.rrel) << " " << c.ubound;
      break;
    case ScaLP::Constraint::type::CEQ:
      os << c.term << " == " << c.ubound;
      break;
    case ScaLP::Constraint::type::C3:
      os << c.lbound << " " << ScaLP::Constraint::showRelation(c.lrel) << " "
         << c.term << " " << ScaLP::Constraint::showRelation(c.rrel) << " " << c.ubound;
      break;
    default: throw ScaLP::Exception("ostream for this Constraint not implemented");

  }
  return os;
}
