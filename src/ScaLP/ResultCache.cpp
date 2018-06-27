#include "ScaLP/ResultCache.h"
#include <fstream>
#include <map>
#include <algorithm>

// directory handling
static bool directoryExists(const std::string& s)
{
  return false;
}

static bool createDirectory(const std::string& d)
{
  // TODO: replace system-call
  system(("mkdir -p \""+d+"\"").c_str());
  return true;
}

bool ScaLP::fileExists(const std::string& s)
{
  std::ifstream f(s.c_str());
  return f.good();
}

static double extractObjective(const std::string& s,double d)
{
  const std::string prefix = "objective value ";
  auto p = s.find(prefix);
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
        m.emplace(std::move(name),value);
        objective = extractObjective(line.substr(pc+1),objective);
      }
      else
      {
        std::string name = line.substr(0,p);
        double value = std::stod(line.substr(p+1));
        m.emplace(std::move(name),value);
      }

    }
  }
  return {m,objective};
}

static ScaLP::Result createResult(const std::pair<std::map<std::string,double>,double>& p,const ScaLP::VariableSet& vs)
{
  ScaLP::Result res;
  res.objectiveValue=p.second;
  for(auto& v:vs)
  {
    auto it = p.first.find(v->getName());
    if(it!=p.first.end())
    {
      double r = p.first.at(v->getName());
      res.values.emplace(v,r);
    }
    else
    {
      //std::cerr << v->getName() << "defaults to 0" << std::endl;
      res.values.emplace(v,0);
    }
  }

  return res;
}

std::pair<bool,double> ScaLP::extractObjective(const std::string& s)
{
  const std::string prefix = "objective value ";
  auto p = s.find(prefix);
  if(p==std::string::npos)
  {
    return {false,0}; // no objective
  }
  else
  {
    return {true,std::stod(s.substr(p+prefix.size()))};
  }
}

bool ScaLP::hasOptimalSolution(const std::string& prefix, const std::string& hash)
{
  return fileExists(prefix+"/"+hash+"/optimal.sol");
}
bool ScaLP::hasFeasibleSolution(const std::string& prefix, const std::string& hash)
{
  return fileExists(prefix+"/"+hash+"/feasible.sol");
}

ScaLP::Result ScaLP::getOptimalSolution(const std::string& prefix, const std::string& hash,const ScaLP::VariableSet& vs)
{
  return createResult(readSolutionFile(prefix+"/"+hash+"/optimal.sol"),vs);
}
ScaLP::Result ScaLP::getFeasibleSolution(const std::string& prefix, const std::string& hash,const ScaLP::VariableSet& vs)
{
  return createResult(readSolutionFile(prefix+"/"+hash+"/feasible.sol"),vs);
}

void ScaLP::writeOptimalSolution(const std::string& prefix, const std::string& hash,ScaLP::Result res, const ScaLP::Solver& solver, bool writeLP)
{
  createDirectory((prefix+"/"+hash));
  std::ofstream s(prefix+"/"+hash+"/optimal.sol");
  s << res.showSolutionVector(true);
  if(writeLP) solver.writeLP(prefix+"/"+hash+"/model.lp");
}
void ScaLP::writeFeasibleSolution(const std::string& prefix, const std::string& hash,ScaLP::Result res, const ScaLP::Solver& solver, bool writeLP)
{
  createDirectory((prefix+"/"+hash));
  std::ofstream s(prefix+"/"+hash+"/feasible.sol");
  s << res.showSolutionVector(true);
  if(writeLP) solver.writeLP(prefix+"/"+hash+"/model.lp");
}
