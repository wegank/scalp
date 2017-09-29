
#include <ScaLP/Objective.h>

using namespace ScaLP;

ScaLP::Objective::Objective(type t,ScaLP::Term term)
  : usedType(t), usedTerm(term)
{
}

Objective ScaLP::minimize(Term term)
{
  return Objective(Objective::type::MINIMIZE,term);
}

Objective ScaLP::maximize(Term term)
{
  return Objective(Objective::type::MAXIMIZE,term);
}

std::ostream& ScaLP::operator<<(std::ostream& os, const ScaLP::Objective& o)
{
  if(o.getType()==ScaLP::Objective::type::MAXIMIZE)
    os << std::string("max(") << o.getTerm() << std::string(")");
  else
    os << std::string("min(") << o.getTerm() << std::string(")");

  return os;
}

ScaLP::Objective::type ScaLP::Objective::getType() const
{
  return this->usedType;
}
const ScaLP::Term &ScaLP::Objective::getTerm() const
{
  return this->usedTerm;
}
