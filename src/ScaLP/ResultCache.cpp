#include "ScaLP/ResultCache.h"
#include <fstream>
#include <map>
#include <algorithm>

// directory handling
static bool directoryExists(const std::string& s)
{
  return false;
}

bool ScaLP::createDirectory(const std::string& d)
{
  return false;
}

bool ScaLP::fileExists(const std::string& s)
{
  std::ifstream f(s.c_str());
  return f.good();
}

static double extractObjective(const std::string& s,double d)
{
  std::string prefix = "objective value ";
  auto p = s.find("objective value ");
  if(p==std::string::npos)
  {
    return d; // no objective
  }
  else
  {
    return std::stod(s.substr(p+prefix.size()));
  }
}

static std::pair<std::map<std::string,double>,double> readSolutionFile(std::string f)
{
  std::map<std::string,double> m;
  double objective=0;

  std::ifstream file(f.c_str());
  
  while(file.good())
  {
    std::string line;
    std::getline(file,line);
    auto p = line.find_first_of(" #");
    if(p==std::string::npos)
    {
      continue; // unrelated (empty) or malformed line: ignore
    }
    else if(line[p]=='#')
    {
      objective = extractObjective(line.substr(p+1),objective);
      continue; // invalid or empty line with comment
    }
    else // ' '
    {
      auto pc = line.find_first_of('#',p);
      if(pc!=std::string::npos)
      { // p==' ', pc=='#'
        std::string name = line.substr(0,p);
        double value = std::stod(line.substr(p+1,pc));
        m.insert({name,value});
        objective = extractObjective(line.substr(pc+1),objective);
      }
      else
      {
        std::string name = line.substr(0,p);
        double value = std::stod(line.substr(p+1));
        m.insert({name,value});
      }

    }
  }
  return {m,objective};
}

static ScaLP::Result createResult(std::pair<std::map<std::string,double>,double> p,const std::vector<ScaLP::Variable>& vs)
{
  ScaLP::Result res;
  res.objectiveValue=p.second;
  for(auto& v:vs)
  {
    double r = p.first[v->getName()];
    res.values.emplace(v,r);
  }

  return res;
}

std::string ScaLP::hashFNV(const std::string& str)
{
    uint64_t base = 14695981039346656037U;
    for(const char c:str)
    {   
        base = (base ^ c) * 1099511628211U;
    }   
    return std::to_string(base);
}

bool ScaLP::hasOptimalSolution(const std::string& prefix, const std::string& hash)
{
  return fileExists(prefix+"/"+hash+"/optimal.sol");
}
bool ScaLP::hasFeasibleSolution(const std::string& prefix, const std::string& hash)
{
  return fileExists(prefix+"/"+hash+"/feasible.sol");
}

ScaLP::Result ScaLP::getOptimalSolution(const std::string& prefix, const std::string& hash,const std::vector<ScaLP::Variable>& vs)
{
  return createResult(readSolutionFile(prefix+"/"+hash+"/optimal.sol"),vs);
}
ScaLP::Result ScaLP::getFeasibleSolution(const std::string& prefix, const std::string& hash,const std::vector<ScaLP::Variable>& vs)
{
  return createResult(readSolutionFile(prefix+"/"+hash+"/feasible.sol"),vs);
}

void ScaLP::writeOptimalSolution(const std::string& prefix, const std::string& hash,ScaLP::Result res, const ScaLP::Solver& solver)
{
  system(("mkdir -p "+prefix+"/"+hash).c_str());
  std::ofstream s(prefix+"/"+hash+"/optimal.sol");
  s << res.showSolutionVector(true);
  solver.writeLP(prefix+"/"+hash+"/model.lp");
}
