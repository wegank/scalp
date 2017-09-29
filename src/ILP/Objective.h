
#pragma once

#include <ILP/Term.h>

namespace ILP
{
  // Users should simply use the ILP::minimize() or ILP::maximize() functions

  class Objective
  {
    public:
      enum class type {MINIMIZE,MAXIMIZE};

      //####################
      // Construction (use minimize or maximize below)
      //####################
      
      Objective() = default;
      Objective(type t,ILP::Term term);
      
      //####################
      // Getter
      //####################
      type getType() const;
      const Term &getTerm() const;

    private:
      // minimize or maximize
      type usedType;

      // the term to minimize or maximize
      Term usedTerm;
  };

  ILP::Objective minimize(ILP::Term term);
  ILP::Objective maximize(ILP::Term term);

  std::ostream& operator<<(std::ostream& os, const ILP::Objective& c);

}

