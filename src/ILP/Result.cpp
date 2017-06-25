
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <ILP/Result.h>

std::string ILP::showStatus(ILP::status s)
{
  switch(s)
  {
    case ILP::status::OPTIMAL:    return "OPTIMAL";
    case ILP::status::FEASIBLE:   return "FEASIBLE";
    case ILP::status::INFEASIBLE: return "INFEASIBLE";
    case ILP::status::UNBOUND:    return "UNBOUND";
    case ILP::status::INVALID:    return "INVALID";
    case ILP::status::ERROR:      return "ERROR";
    case ILP::status::TIMEOUT:    return "TIMEOUT";
    case ILP::status::NOT_SOLVED: return "NOT_SOLVED";
  }
  return "UNKNOWN";
}

std::ostream& ILP::operator<<(std::ostream& os, const ILP::Result &r)
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
std::ostream& ILP::operator<<(std::ostream& os, const ILP::status &s)
{
  return os << showStatus(s);
}

std::string ILP::Result::showSolutionVector(bool compact)
{
  std::stringstream ss;
  ss << "# objective value " << this->objectiveValue << "\n";
  ss << std::left;
  for(auto p:this->values)
  {
    if((compact and p.second!=0) or (not compact)) ss << std::setw(compact?0:8) << p.first->name << " " << p.second << "\n"; 
  }

  return ss.str();

}

void ILP::Result::writeSolutionVector(std::string file, bool compact)
{
  std::ofstream ss(file);
  ss << this->showSolutionVector(compact);
  ss.flush();
}
