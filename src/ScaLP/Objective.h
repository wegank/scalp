
#pragma once

#include <ScaLP/Term.h>

namespace ScaLP
{
  // Users should simply use the ScaLP::minimize() or ScaLP::maximize() functions

  class Objective
  {
    public:
      enum class type {MINIMIZE,MAXIMIZE};

      //####################
      // Construction (use minimize or maximize below)
      //####################
      
      Objective() = default;
      Objective(type t,ScaLP::Term term);
      
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

  ScaLP::Objective minimize(ScaLP::Term term);
  ScaLP::Objective maximize(ScaLP::Term term);

  std::ostream& operator<<(std::ostream& os, const ScaLP::Objective& c);

}

