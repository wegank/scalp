
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ScaLP/Result.h>

std::string ScaLP::showStatus(ScaLP::status s)
{
  switch(s)
  {
    case ScaLP::status::OPTIMAL:    return "OPTIMAL";
    case ScaLP::status::FEASIBLE:   return "FEASIBLE";
    case ScaLP::status::INFEASIBLE: return "INFEASIBLE";
    case ScaLP::status::UNBOUND:    return "UNBOUND";
    case ScaLP::status::INVALID:    return "INVALID";
    case ScaLP::status::ERROR:      return "ERROR";
    case ScaLP::status::NOT_SOLVED: return "NOT_SOLVED";
    case ScaLP::status::TIMEOUT_FEASIBLE: return "TIMEOUT_FEASIBLE";
    case ScaLP::status::TIMEOUT_INFEASIBLE: return "TIMEOUT_INFEASIBLE";
    case ScaLP::status::INFEASIBLE_OR_UNBOUND: return "INFEASIBLE_OR_UNBOUND";
    case ScaLP::status::UNKNOWN:    return "UNKNOWN";
  }
  return "UNKNOWN";
}

std::ostream& ScaLP::operator<<(std::ostream& os, const ScaLP::Result &r)
{
  os << "Objective: " << r.objectiveValue << std::endl;
  os << "Variables:" << std::endl;
  for(auto &p:r.values)
  {
    os << "  " << std::left << std::setw(8) << p.first << std::setw(7) << " = " << p.second << std::endl;
  } 
  os << "Durations:" << std::endl;
  os << "  preparation:  " << r.preparationTime << std::endl;
  os << "  construction: " << r.constructionTime << std::endl;
  os << "  solving:      " << r.solvingTime << std::endl;
  return os;
}
std::ostream& ScaLP::operator<<(std::ostream& os, const ScaLP::status &s)
{
  return os << showStatus(s);
}

std::string ScaLP::Result::showSolutionVector(bool compact)
{
  std::ostringstream ss;
  ss << "# objective value " << this->objectiveValue << "\n";
  ss << std::left;
  for(auto p:this->values)
  {
    if((compact and p.second!=0) or (not compact)) ss << std::setw(compact?0:8) << p.first->getName() << " " << p.second << "\n"; 
  }

  return ss.str();

}

void ScaLP::Result::writeSolutionVector(std::string file, bool compact)
{
  std::ofstream ss(file);
  ss << this->showSolutionVector(compact);
  ss.flush();
}
