
#pragma once

#include <ILP/Term.h>

namespace ILP
{
  // Users should simply use the ILP::minimize() or ILP::maximize() functions

  class Objective
  {
    public:
      enum class type {MINIMIZE,MAXIMIZE};

      Objective() = default;
      Objective(type t,ILP::Term term);

      // minimize or maximize
      type usedType;

      // the term to minimize or maximize
      Term usedTerm;

    private:
  };

  ILP::Objective minimize(ILP::Term term);
  ILP::Objective maximize(ILP::Term term);

  std::ostream& operator<<(std::ostream& os, const ILP::Objective& c);

}

