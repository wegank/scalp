
#include <ILP/Objective.h>

using namespace ILP;

ILP::Objective::Objective(type t,ILP::Term term)
  : usedType(t), usedTerm(term)
{
}

Objective ILP::minimize(Term term)
{
  return Objective(Objective::type::MINIMIZE,term);
}

Objective ILP::maximize(Term term)
{
  return Objective(Objective::type::MAXIMIZE,term);
}

std::ostream& ILP::operator<<(std::ostream& os, const ILP::Objective& o)
{
  if(o.getType()==ILP::Objective::type::MAXIMIZE)
    os << std::string("max(") << o.getTerm() << std::string(")");
  else
    os << std::string("min(") << o.getTerm() << std::string(")");

  return os;
}

ILP::Objective::type ILP::Objective::getType() const
{
  return this->usedType;
}
const ILP::Term &ILP::Objective::getTerm() const
{
  return this->usedTerm;
}
