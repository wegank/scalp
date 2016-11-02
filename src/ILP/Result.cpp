
#include <iostream>
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

std::ostream& operator<<(std::ostream& os, const ILP::Result &r)
{
  for(auto &p:r.values)
  {
    std::cout << p.first << "=" << p.second << std::endl;
  } 
  return os;
}
std::ostream& operator<<(std::ostream& os, const ILP::status &s)
{
  return os << showStatus(s);
}
