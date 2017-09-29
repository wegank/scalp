#pragma once

#include <map>
#include <string>

#include <ScaLP/Variable.h>

namespace ScaLP
{
  enum class status
  { NOT_SOLVED
  , OPTIMAL
  , FEASIBLE
  , INFEASIBLE
  , INFEASIBLE_OR_UNBOUND
  , UNBOUND
  , INVALID
  , ERROR
  , TIMEOUT
  , UNKNOWN
  };

  std::string showStatus(ScaLP::status s);

  class Result
  {
    public:

      double objectiveValue=0;

      std::map<ScaLP::Variable,double> values;

      double preparationTime=0;
      double constructionTime=0;
      double solvingTime=0;

      std::string showSolutionVector(bool compact=false);
      void writeSolutionVector(std::string file, bool compact=false);

    private:
  };
  std::ostream& operator<<(std::ostream& os, const ScaLP::Result &r);
  std::ostream& operator<<(std::ostream& os, const ScaLP::status &s);
}

