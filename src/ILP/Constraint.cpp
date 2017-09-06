
#include <ILP/Constraint.h>
#include <ILP/Exception.h>

namespace ILP
{
  extern double INF();
}

ILP::Constraint::~Constraint()
{
}

static bool isIndicator(const ILP::Constraint& c)
{
  return c.ctype==ILP::Constraint::type::CEQ // ... == ...
    and c.lbound==c.ubound and (c.lbound == 0 or c.lbound == 1) // ... == 1 or 0
    and (c.term.sum.size()==1 and c.term.constant==0) // x == [1,0]
    and c.term.sum.begin()->first->usedType==ILP::VariableType::BINARY
    and c.term.sum.begin()->second==1; // 1*x(binary) == 1 or 0
}

ILP::Constraint::Constraint(ILP::Constraint i, ILP::Constraint c)
: Constraint(c)
{
  if(isIndicator(i))
  {
    this->indicator = std::make_shared<Constraint>(i);
  }
  else
  {
    throw ILP::Exception("The used constraint is not an indicator. (binary variable == 0 or 1)");
  }
}


std::string ILP::Constraint::showRelation(relation r)
{
  switch(r)
  {
    case ILP::relation::LESS_EQ_THAN: return "<=";
    case ILP::relation::MORE_EQ_THAN: return ">=";
    case ILP::relation::EQUAL: return "=";
    default:return "";
  }
  return "";
}

ILP::VariableSet ILP::Constraint::extractVariables() const
{
  ILP::VariableSet s{term.extractVariables()};
  if(this->indicator!=nullptr)
  {
    ILP::VariableSet t{this->indicator->term.extractVariables()};
    s.insert(t.begin(),t.end());
  }
  return s;
}

std::string ILP::Constraint::show() const
{
  //TODO:
  throw ILP::Exception("This Constraint-type is not implemented yet.");
  return "";
}

static bool relationsNotCompatible(ILP::relation l, ILP::relation r)
{
  using rel = ILP::relation;
  return (l==rel::EQUAL or r==rel::EQUAL) // a == b == c, a <= b == c (does not make much sense?)
    or (l==rel::MORE_EQ_THAN and r==rel::LESS_EQ_THAN) // a >= b <= c (two upper bounds)
    or (l==rel::LESS_EQ_THAN and r==rel::MORE_EQ_THAN) // a <= b >= c (two lower bounds)
    ;
}

static inline void checkRelationCompatibility(ILP::relation l, ILP::relation r)
{
  if(relationsNotCompatible(l,r))
  {
    throw ILP::Exception("The used relations ("+ILP::Constraint::showRelation(l)+") and ("+ILP::Constraint::showRelation(r)+") are not compatible");
  }
}

// basic constructors
ILP::Constraint::Constraint(double l, relation rel, ILP::Term& r)
  : ctype(ILP::Constraint::type::C2L), lbound(l), lrel(rel), term(r)
{
  if(rel==ILP::relation::EQUAL)
  {
    ctype=ILP::Constraint::type::CEQ;
    rrel=ILP::relation::EQUAL;
    ubound=lbound;
  }
}
ILP::Constraint::Constraint(ILP::Term& l, relation rel, double r)
  : ctype(ILP::Constraint::type::C2R), term(l), rrel(rel), ubound(r)
{
  if(rel==ILP::relation::EQUAL)
  {
    ctype=ILP::Constraint::type::CEQ;
    lrel=ILP::relation::EQUAL;
    lbound=ubound;
  }
}
ILP::Constraint::Constraint(double lb, relation lrel, ILP::Term& t, relation rrel,double ub)
  : ctype(ILP::Constraint::type::C3), lbound(lb), lrel(lrel), term(t), rrel(rrel), ubound(ub)
{
  checkRelationCompatibility(lrel,rrel);
}

// combination constructors
ILP::Constraint::Constraint(ILP::Constraint& lhs, relation rel, double ub)
  : Constraint(lhs.lbound,lhs.lrel,lhs.term,rel,ub)
{
}
ILP::Constraint::Constraint(double lb, relation rel, ILP::Constraint& rhs)
  : Constraint(lb,rel,rhs.term,rhs.rrel,rhs.ubound)
{
}

// named constraint constructors
ILP::Constraint::Constraint(std::string n, ILP::Constraint& c)
  : ILP::Constraint(c)
{
  name=n;
}
ILP::Constraint::Constraint(std::string n, ILP::Constraint&& c)
  : ILP::Constraint(c)
{
  name=n;
}
ILP::Constraint::Constraint(std::pair<std::string,ILP::Constraint>& p)
  : ILP::Constraint(p.second)
{
  name=p.first;
}

std::ostream& ILP::operator<<(std::ostream& os, const ILP::Constraint &c)
{
  switch(c.ctype)
  {
    case ILP::Constraint::type::C2L:
      os << c.lbound << " " << ILP::Constraint::showRelation(c.lrel) << " " << c.term;
      break;
    case ILP::Constraint::type::C2R:
      os << c.term << " " << ILP::Constraint::showRelation(c.rrel) << " " << c.ubound;
      break;
    case ILP::Constraint::type::CEQ:
      os << c.term << " == " << c.ubound;
      break;
    case ILP::Constraint::type::C3:
      os << c.lbound << " " << ILP::Constraint::showRelation(c.lrel) << " "
         << c.term << " " << ILP::Constraint::showRelation(c.rrel) << " " << c.ubound;
      break;
    default: throw ILP::Exception("ostream for this Constraint not implemented");

  }
  return os;
}
