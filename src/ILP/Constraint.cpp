
#include <ILP/Constraint.h>
#include <ILP/Exception.h>

namespace ILP
{
  extern double INF();
}

std::string ILP::Constraint::showRelation(relation r)
{
  switch(r)
  {
    case ILP::relation::LESS_EQ_THAN: return "<=";
    case ILP::relation::LESS_THAN: return "<";
    case ILP::relation::MORE_EQ_THAN: return ">=";
    case ILP::relation::MORE_THAN: return ">";
    case ILP::relation::EQUAL: return "==";
    default:return "";
  }
  return "";
}

ILP::VariableSet ILP::Constraint::extractVariables() const
{
  ILP::VariableSet vs;
  if(usedType==ILP::Constraint::type::Constraint_2)
  {
    ILP::VariableSet lvs = c2.lhs.extractVariables();
    vs.insert(lvs.begin(),lvs.end());
    ILP::VariableSet rvs = c2.rhs.extractVariables();
    vs.insert(rvs.begin(),rvs.end());
  }
  else if(usedType==ILP::Constraint::type::Constraint_3)
  {
    ILP::VariableSet vvs = c3.term.extractVariables();
    vs.insert(vvs.begin(),vvs.end());
  }
  else throw ILP::Exception("This Constraint-type is not implemented yet.");
  return vs;
}

std::string ILP::Constraint::show() const
{
  if(usedType==ILP::Constraint::type::Constraint_2)
  {
    return ""; // TODO
  }
  else if(usedType==ILP::Constraint::type::Constraint_3)
  {
    return ""; // TODO
  }
  else throw ILP::Exception("This Constraint-type is not implemented yet.");
}

static bool relationsNotCompatible(ILP::relation l, ILP::relation r)
{
  using rel = ILP::relation;
  return (l==rel::EQUAL or r==rel::EQUAL) // a == b == c, a <= b == c (does not make much sense?)
    or ((l==rel::MORE_EQ_THAN or l==rel::MORE_THAN) and (r==rel::LESS_EQ_THAN or r==rel::LESS_THAN)) // a > b < c (two upper bounds)
    or ((l==rel::LESS_EQ_THAN or l==rel::LESS_THAN) and (r==rel::MORE_EQ_THAN or r==rel::MORE_THAN)) // a < b > c (two lower bounds)
    ;
}

static inline void checkRelationCompatibility(ILP::relation l, ILP::relation r)
{
  if(relationsNotCompatible(l,r))
  {
    throw ILP::Exception("The used relations ("+ILP::Constraint::showRelation(l)+") and ("+ILP::Constraint::showRelation(r)+") are not compatible");
  }
}

ILP::Constraint::Constraint(ILP::Term l, relation rel, ILP::Term r)
  : usedType(ILP::Constraint::type::Constraint_2)
  , c2(l,rel,r)
{
}

ILP::Constraint::Constraint(ILP::Term lb, relation lrel, ILP::Term t, relation rrel,ILP::Term ub)
  : usedType(ILP::Constraint::type::Constraint_3)
  , c3(lb.constant,lrel,t,rrel,ub.constant)
{
}

ILP::Constraint::Constraint(ILP::Constraint3 c)
  : usedType(ILP::Constraint::type::Constraint_3), c3(c)
{
}
ILP::Constraint::Constraint(ILP::Constraint2 c)
  : usedType(ILP::Constraint::type::Constraint_2), c2(c)
{
}

ILP::Constraint::Constraint(ILP::Constraint c, relation rel, ILP::Term ub)
{ 
  if(c.usedType!=ILP::Constraint::type::Constraint_2) throw ILP::Exception("Can't combine three relations.");
  ILP::Constraint2 &lhs=c.c2;

  usedType=ILP::Constraint::type::Constraint_3;
  c3 = ILP::Constraint3(lhs.lhs.constant,lhs.usedRelation,lhs.rhs,rel,ub);
}
ILP::Constraint::Constraint(ILP::Term lb, relation rel, ILP::Constraint c)
{
  if(c.usedType!=ILP::Constraint::type::Constraint_2) throw ILP::Exception("Can't combine three relations.");
  ILP::Constraint2 &rhs=c.c2;

  usedType=ILP::Constraint::type::Constraint_3;
  c3 = ILP::Constraint3(lb,rel,rhs.lhs,rhs.usedRelation,rhs.rhs.constant);
}

ILP::Constraint2::Constraint2(ILP::Term l, ILP::relation rel, ILP::Term r)
  : lhs(l), usedRelation(rel), rhs(r)
{
  // if(not l.isConstant() && not r.isConstant()) throw ILP::Exception("At least one side of the relation ("+ILP::Constraint::showRelation(rel)+") needs to be constant.");
}

ILP::Constraint3::Constraint3(Term lb, ILP::relation lrel, ILP::Term t, ILP::relation rrel,Term ub)
  : lbound(lb), lrel(lrel), term(t), rrel(rrel), ubound(ub)
{
  // // check for constant bounds
   if(not lb.isConstant()) throw ILP::Exception("The lower bound is not constant.");
   if(not ub.isConstant()) throw ILP::Exception("The upper bound is not constant.");

  checkRelationCompatibility(lrel,rrel);

  this->lbound = lb.constant;
  this->lrel=lrel;
  this->term=t;
  this->rrel=rrel;
  this->ubound = ub.constant;
}


ILP::Constraint3::Constraint3()
  : lbound(-ILP::INF())
  , lrel(ILP::relation::LESS_EQ_THAN)
  , rrel(ILP::relation::LESS_EQ_THAN)
  , ubound(ILP::INF())
{
}

std::ostream& operator<<(std::ostream& os, const ILP::Constraint &c)
{
  if(c.usedType==ILP::Constraint::type::Constraint_2)
    os << c.c2;
  else if(c.usedType==ILP::Constraint::type::Constraint_3)
    os << c.c3;
  else throw ILP::Exception("ostream for this Constraint not implemented");

  return os;
}
std::ostream& operator<<(std::ostream& os, const ILP::Constraint2 &c)
{
  os << c.lhs << " " << ILP::Constraint::showRelation(c.usedRelation) << " " << c.rhs;
  return os;
}
std::ostream& operator<<(std::ostream& os, const ILP::Constraint3 &c)
{
  os << c.lbound << " " << ILP::Constraint::showRelation(c.lrel) << " ";
  os << c.term;
  os << " " << ILP::Constraint::showRelation(c.lrel) << c.ubound;
  return os;
}
