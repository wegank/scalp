
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

std::ostream& operator<<(std::ostream& os, const ILP::Objective &o)
{
  if(o.usedType==ILP::Objective::type::MAXIMIZE)
    os << std::string("max(") << o.usedTerm << std::string(")");
  else
    os << std::string("min(") << o.usedTerm << std::string(")");

  return os;
}
