#pragma once

#include <map>
#include <string>

#include <ILP/Variable.h>

namespace ILP
{
  enum class status
  { NOT_SOLVED
  , OPTIMAL
  , FEASIBLE
  , INFEASIBLE
  , UNBOUND
  , INVALID
  , ERROR
  , TIMEOUT
  };

  std::string showStatus(ILP::status s);

  class Result
  {
    public:

      double objectiveValue=0;

      std::map<ILP::Variable,double> values;

    private:
  };
}

std::ostream& operator<<(std::ostream& os, const ILP::Result &r);
std::ostream& operator<<(std::ostream& os, const ILP::status &s);
