
#include <limits>
#include <sstream>
#include <fstream>
#include <ctime>
#include <cmath>
#include <initializer_list>
#include <functional>

#include <ScaLP/Exception.h>
#include <ScaLP/Solver.h>
#include <ScaLP/Result.h>
#include <ScaLP/SolverBackend/SolverDynamic.h>
#include <ScaLP/ResultCache.h>
#ifdef LP_PARSER
#include "parse.h"
#endif

#include <iostream>

static double plus(double a, double b)
{
  return a+b;
}

double ScaLP::INF()
{
  return std::numeric_limits<double>::infinity();
}

ScaLP::Solver::Solver(ScaLP::SolverBackend *b)
  :back(b)
{
}

ScaLP::Solver::Solver(std::list<std::string> ls)
  :back(newSolverDynamic(ls))
{
}
ScaLP::Solver::Solver(std::list<ScaLP::Feature>fs, std::list<std::string> ls)
  :back(newSolverDynamic(fs,ls))
{
}
ScaLP::Solver::Solver(std::initializer_list<std::string> ls)
  :back(newSolverDynamic(ls))
{
}

ScaLP::Solver::~Solver()
{
  if(this->back!=nullptr) delete back;
}

void ScaLP::Solver::setBackend(SolverBackend *b)
{
  if(this->back!=nullptr) delete this->back;
  this->back=b;
}

ScaLP::SolverBackend* ScaLP::Solver::releaseSolver()
{
  auto* p = this->back;
  this->back=nullptr;
  return p;
}

bool ScaLP::Solver::load(const std::string& file)
{
#ifdef LP_PARSER
  return ScaLP::ParserLP::parse(file,*this);
#else
  return false;
#endif
}

void ScaLP::Solver::setConstraintCount(unsigned int n)
{
  cons.reserve(n);
}

// extract the Variables from the Constraints and the Objective to avoid unused
// variables.
static ScaLP::VariableSet extractVariables(const std::vector<ScaLP::Constraint> &cs,const ScaLP::Objective &o)
{
  ScaLP::VariableSet vs = o.getTerm().extractVariables();

  for(auto& c:cs)
  {
    ScaLP::VariableSet vvs = c.extractVariables();
    vs.insert(vvs.begin(),vvs.end());
  }

  return vs;
}

void ScaLP::Solver::setObjective(const Objective& o)
{
  this->modelChanged=true;
  this->objective=o;

  // this should throw an exception if the Objective rises a name-collision
  extractVariables(cons,objective);
}

static ScaLP::relation flipRelation(ScaLP::relation r)
{
  using R = ScaLP::relation;
  switch(r)
  {
    case R::LESS_EQ_THAN: return R::MORE_EQ_THAN;
    case R::MORE_EQ_THAN: return R::LESS_EQ_THAN;
    default: return R::EQUAL;
  }
  return R::EQUAL;
}

static void normalizeConstraint(ScaLP::Constraint& c)
{
  // remove constant from Term
  if(c.term.constant!=0)
  {
    c.lbound-=c.term.constant;
    c.ubound-=c.term.constant;
    c.term.constant=0;
  }

  // flip Relation to get the constant to the right
  if(c.ctype==ScaLP::Constraint::type::C2L)
  {
    c.ubound = c.lbound;
    c.lbound = 0;
    c.rrel   = flipRelation(c.lrel);
    c.ctype  = ScaLP::Constraint::type::C2R;
  }
}

static void constraintFeatureGuard(ScaLP::SolverBackend* back, const ScaLP::Constraint& c)
{
  if(back!=nullptr)
  {
    if(c.indicator!=nullptr and not back->features.indicators)
    {
      throw ScaLP::Exception("This ScaLP-backend \""+back->name+"\" does not support indicator-constraints");
    }

  }

}

void ScaLP::Solver::addConstraint(Constraint& b)
{
  constraintFeatureGuard(this->back,b);
  normalizeConstraint(b);
  this->cons.emplace_back(b);
  modelChanged=true;
}
void ScaLP::Solver::addConstraint(Constraint&& b)
{
  constraintFeatureGuard(this->back,b);
  normalizeConstraint(b);
  this->cons.emplace_back(b);
  modelChanged=true;
}

static std::string showTermLP(const ScaLP::Term& t)
{

  // only constant
  if(t.isConstant())
  {
    return std::to_string(t.constant);
  }

  std::string s;

  // TODO: this is a hotfix
  const std::map<ScaLP::Variable,double,ScaLP::variableComparator> tt(t.sum.begin(),t.sum.end());

  bool first=true; // first iteration
  for(const auto &p:tt)
  {
    // eliminated Variable
    if(p.second==0) continue;

    // show coefficient?
    if(p.second!=1 && p.second!=-1)
    {
      if(!first)
      {
        if(p.second<0)
        {
          s += " - ";
          s += std::to_string(-p.second);
        }
        else
        {
          s += " + ";
          s += std::to_string(p.second);
        }
      }
      else
      {
        if(p.second<0)
        {
          s += "-";
          s += std::to_string(-p.second);
          s += " ";
        }
        else
        {
          s += std::to_string(p.second);
          s += " ";
        }
      }
    }

    // signed variable?
    if(p.second==-1)
    {
      if(first)
      {
        s += "-";
      }
      else
      {
        s += " -";
      }
    }

    if(p.second==1 && !first)
      s += " +";
      
    if(!first) s += " ";

    s += p.first->getName();
    first=false;
  }

  // constant part
  if(t.constant!=0)
  {
    if(t.constant<0)
    {
      s += " - ";
      s += std::to_string(-t.constant);
    }
    else
    {
      s += " + ";
      s += std::to_string(t.constant);
    }
  }

  return s;
}

static std::string showObjectiveLP(const ScaLP::Objective& o)
{
  std::string s;
  if(o.getType()==ScaLP::Objective::type::MAXIMIZE)
  {
    s="MAXIMIZE";
  }
  else
  {
    s="MINIMIZE";
  }
  s += "\n  ";
  s += showTermLP(o.getTerm());
  s += "\n";

  return s;
}

static std::string showConstraint2LP(const ScaLP::Term& lhs,ScaLP::relation rel, const ScaLP::Term& rhs)
{
  std::string s = showTermLP(lhs);
  s += " ";
  s += ScaLP::Constraint::showRelation(rel);
  s += " "; 
  s += showTermLP(rhs);
  return s;
}

static std::string showConstraint3LP(const ScaLP::Constraint& c)
{
  std::string s = showTermLP(c.lbound);
  s += " ";
  s += ScaLP::Constraint::showRelation(c.lrel);
  s += " ";
  s += showTermLP(c.term);
  s += " ";
  s += ScaLP::Constraint::showRelation(c.rrel);
  s += " ";
  s += showTermLP(c.ubound);
  return s;
}

static std::string showConstraintLP(const ScaLP::Constraint& c)
{
  std::string prefix="";

  if(c.name!="")
  {
    prefix = c.name+": ";
  }

  if(c.indicator!=nullptr)
  {
    prefix += showConstraint2LP(c.indicator->term,c.indicator->lrel,c.indicator->lbound)+ " -> ";
  }

  switch(c.ctype)
  {
    case ScaLP::Constraint::type::C2L: 
      return prefix+showConstraint2LP(c.term,flipRelation(c.lrel),c.lbound);
    case ScaLP::Constraint::type::C2R:
      return prefix+showConstraint2LP(c.term,c.rrel,c.ubound);
    case ScaLP::Constraint::type::CEQ:
      return prefix+showConstraint2LP(c.term,c.lrel,c.lbound);
    case ScaLP::Constraint::type::C3:
      return prefix+showConstraint3LP(c);
  }
  return "";
}

static std::string variableTypesLP(const ScaLP::VariableSet& vs)
{
  std::string binary="BINARY\n";
  std::string general="GENERAL\n";
  for(const ScaLP::Variable& v:vs)
  {
    if(v->getType()==ScaLP::VariableType::INTEGER)
      general+="  "+v->getName()+"\n";
    else if(v->getType()==ScaLP::VariableType::BINARY)
      binary+="  "+v->getName()+"\n";

  }
  return binary+general;
}

static std::string boundsLP(const ScaLP::VariableSet& vs)
{
  std::ostringstream s;
  for(const auto &v:vs)
  {
    auto lb   = v->getLowerBound();
    auto ub   = v->getUpperBound();
    auto name = v->getName();

    // default
    if(lb==0 and ub==ScaLP::INF())
    {
      continue;
    }

    // indentation
    s << "  ";

    // free
    if(lb==-ScaLP::INF() and ub==ScaLP::INF())
    {
      s << name;
      s << " FREE";
    }

    // x <= b
    else if(lb==0 and ub!=ScaLP::INF())
    {
      s << name;
      s << " <= ";
      s << ub;
    }

    // a <= x
    else if(lb!=0 && ub==ScaLP::INF())
    {
      s << lb;
      s << " <= ";
      s << name;
    }

    // a <= x <= b
    else
    {
      s << lb;
      s << " <= ";
      s << name;
      s << " <= ";
      s << ub;
    }

    // end of entry
    s << "\n";

  }
  return s.str();
}

std::string ScaLP::Solver::getBackendName() const
{
  return back->name;
}

static void showLPBase(const std::function<void(std::string)>& f
  , const ScaLP::Objective& objective
  , const std::vector<ScaLP::Constraint>& cons
  , const ScaLP::VariableSet& vs)
{
  f(showObjectiveLP(objective));

  f("SUBJECT TO\n");
  for(auto &c:cons)
  {
    f("  "+showConstraintLP(c)+"\n");
  }

  f("BOUNDS\n");
  f(boundsLP(vs));

  f(variableTypesLP(vs));

  f("END\n");
}

std::string ScaLP::Solver::showLP() const
{
  std::string s;

  auto f = [&s](const std::string& str)
  {
    s+=str;
  };
  showLPBase(f,objective,cons,extractVariables(cons,objective));

  return s;
}

void ScaLP::Solver::writeLP(std::string file) const
{
  std::ofstream s(file);

  auto f = [&s](const std::string& str)
  {
    s<<str;
  };
  showLPBase(f,objective,cons,extractVariables(cons,objective));
}
void ScaLP::Solver::writeLP(std::string file, const ScaLP::VariableSet& vs) const
{
  std::ofstream s(file);

  auto f = [&s](const std::string& str)
  {
    s<<str;
  };
  showLPBase(f,objective,cons,vs);
}

void ScaLP::Solver::prepare()
{
  // set the used threads
  if(threads>0) back->setThreads(threads);

  // set the verbosity of the backend
  back->setConsoleOutput(!quiet);

  if(timeout>0) back->setTimeout(timeout);

  if(intFeasTol>=0) back->setIntFeasTol(intFeasTol);

  if(absMIPGap>=0) back->setAbsoluteMIPGap(absMIPGap);
  if(relMIPGap>=0) back->setRelativeMIPGap(relMIPGap);

  // use presolve?
  back->presolve(presolve);
}

static void construction(ScaLP::SolverBackend* back, const ScaLP::VariableSet& vs, const ScaLP::Objective& obj, const std::vector<ScaLP::Constraint>& cons)
{
  // Add the Variables
  // Only add the used Variables.
  back->addVariables(vs);

  // Add Objective
  back->setObjective(obj);

  // Add Constraints
  back->addConstraints(cons);
}
static void construction(ScaLP::SolverBackend* back, const ScaLP::VariableSet& vs, const ScaLP::Objective& obj, const std::vector<ScaLP::Constraint>& cons, ScaLP::Result& start)
{
  construction(back,vs,obj,cons);
  if(start.empty())
  {
    for(auto&p:vs)
    {
      if(p->getStart()!=NAN)
      {
        start.values.emplace(p,p->getStart());
      }
    }
  }
  if(not start.empty()) back->setStartValues(start);
}

void ScaLP::Solver::construct(const ScaLP::VariableSet& vs)
{
  if(warmStart) construction(back,vs,objective,cons,warmStartValues);
  else construction(back,vs,objective,cons);
}
void ScaLP::Solver::construct()
{
  auto vs = extractVariables(cons,objective);
  if(warmStart) construction(back,vs,objective,cons,warmStartValues);
  else construction(back,vs,objective,cons);
}
void ScaLP::Solver::construct(const std::string& file)
{
  throw ScaLP::Exception("not implemented yet.");
}

void ScaLP::Solver::postprocess()
{
  // round integer values
  for(auto&p:this->result.values)
  {
    if(p.first->getType()==ScaLP::VariableType::INTEGER or p.first->getType()==ScaLP::VariableType::BINARY)
    {
      p.second = std::lround(p.second);
    }
  }
}

template <class F>
double time(const F& f)
{
  auto timer = std::clock();
  f();
  return (double(std::clock()-timer))/CLOCKS_PER_SEC;
}

ScaLP::status ScaLP::Solver::newSolve()
{
  back->reset();

  ScaLP::Result res= ScaLP::Result();
  ScaLP::status stat;

  double preparationTime = time([this](){prepare();});
  double constructionTime = time([&,this](){construct();});
  double solvingTime = time([&stat,&res,this](){
    std::tie(stat,res) = back->solve();
  });

  res.preparationTime = preparationTime;
  res.constructionTime = constructionTime;
  res.solvingTime = solvingTime;

  // round integer-values in the result
  postprocess();

  this->result = res;
  return stat;
}

ScaLP::status ScaLP::Solver::newSolve(const ScaLP::VariableSet& vs)
{
  back->reset();

  ScaLP::Result res= ScaLP::Result();
  ScaLP::status stat;

  double preparationTime = time([this](){prepare();});
  double constructionTime = time([&,this,vs](){construct(vs);});
  double solvingTime = time([&stat,&res,this](){
    std::tie(stat,res) = back->solve();
  });

  res.preparationTime = preparationTime;
  res.constructionTime = constructionTime;
  res.solvingTime = solvingTime;

  // round integer-values in the result
  postprocess();

  this->result = res;
  return stat;
}

static std::string hashFNV(const ScaLP::Objective& objective
  , const std::vector<ScaLP::Constraint>& cons
  , const ScaLP::VariableSet& vs)
{
  std::vector<uint64_t> hashBases(3,14695981039346656037U);
  std::string reminder="";
  auto hashFNVH = [&hashBases,&reminder](const std::string& str)
  {
    std::string acc = reminder+str;
    size_t dataSize=acc.size()-acc.size()%hashBases.size();
    reminder = acc.substr(dataSize);
    acc.resize(dataSize);
    for(unsigned int i=0;i<dataSize;i+=hashBases.size())
    {
      for(unsigned int j=0;j<hashBases.size();++j)
      {
        hashBases[j]= (hashBases[j]^acc[i+j])*1099511628211U;
      }
    }
  };
  
  showLPBase(hashFNVH,objective,cons,vs);
  
  // hash the reminder
  for(unsigned int i=0;i<reminder.size();++i)
  {
    hashBases[i]= (hashBases[i]^reminder[i])*1099511628211U;
  }
  
  // combine all hashes
  std::string hash="";
  for(const auto& i:hashBases)
  {
    hash+=std::to_string(i);
  }
  return hash;
}

static void updateCache(ScaLP::Solver& solver,const ScaLP::Result& result, const ScaLP::Objective objective, const std::string& hash, const std::string& cacheDir, bool writeLP)
{
  std::ifstream f((cacheDir+"/"+hash+"/feasible.sol").c_str());
  while(f.good())
  {
    std::string line="";
    std::getline(f,line);
    auto p = ScaLP::extractObjective(line);
    if(p.first) // extracted
    {
      if(objective.getType()==ScaLP::Objective::type::MAXIMIZE)
      {
        if(result.objectiveValue>p.second)
        {
          ScaLP::writeFeasibleSolution(cacheDir,hash,result,solver,writeLP);
          break;
        }
      }
      else
      {
        if(result.objectiveValue<p.second)
        {
          ScaLP::writeFeasibleSolution(cacheDir,hash,result,solver,writeLP);
          break;
        }
      }
    }
  }
}

ScaLP::status ScaLP::Solver::solve()
{
  if(not this->modelChanged) return ScaLP::status::ALREADY_SOLVED;
  else this->modelChanged=false;

  const ScaLP::VariableSet s = extractVariables(cons,objective);

  if(not resultCacheDir.empty())
  {
    std::cerr << "ScaLP: ScaLP::Solver::resultCacheDir is obsolete, please use ScaLP::Solver::resultCache.directory instead" << std::endl;
    resultCache.directory = resultCacheDir;
  }

  if(not resultCache.directory.empty())
  {
    std::string hash=hashFNV(objective,cons,s);
    if(ScaLP::hasOptimalSolution(resultCache.directory,hash))
    {
      this->result = ScaLP::getOptimalSolution(resultCache.directory,hash,s);
      return ScaLP::status::OPTIMAL;
    }
    else
    {
      if(resultCache.preferCachedValues and ScaLP::hasFeasibleSolution(resultCache.directory,hash))
      {
        this->result = ScaLP::getFeasibleSolution(resultCache.directory,hash,s);
        return ScaLP::status::FEASIBLE;
      }
      auto stat = newSolve(s);
      if(stat==ScaLP::status::OPTIMAL)
      {
        ScaLP::writeOptimalSolution(resultCache.directory,hash,this->result,*this,resultCache.addModel);
      }
      else if(stat==ScaLP::status::FEASIBLE or stat==ScaLP::status::TIMEOUT_FEASIBLE)
      {
        if(not ScaLP::hasFeasibleSolution(resultCache.directory,hash))
        {
          ScaLP::writeFeasibleSolution(resultCache.directory,hash,this->result,*this,resultCache.addModel);
        }
        else
        {
          updateCache(*this,this->result,this->objective,hash,resultCache.directory,resultCache.addModel);
        }
      }
      else if(stat==ScaLP::status::TIMEOUT_INFEASIBLE)
      {
        if(ScaLP::hasFeasibleSolution(resultCache.directory,hash))
        {
          this->result = ScaLP::getFeasibleSolution(resultCache.directory,hash,s);
          return ScaLP::status::TIMEOUT_FEASIBLE;
        }
      }
      return stat;
    }
  }
  else
  {
    return newSolve(s);
  }
}

ScaLP::status ScaLP::Solver::solve(const ScaLP::Result& start)
{
  this->warmStartValues=start;
  return solve();
}

ScaLP::status ScaLP::Solver::solve(const std::string& file)
{
  // reset the backend
  back->reset();

  ScaLP::status stat;
  ScaLP::Result res= ScaLP::Result();
  
  double preparationTime = time([this](){prepare();});
  double constructionTime = time([&,this](){construct(file);});
  double solvingTime = time([&stat,&res,this](){
    std::tie(stat,res) = back->solve();
  });

  result.preparationTime = preparationTime;
  result.constructionTime = constructionTime;
  result.solvingTime = solvingTime;

  // round integer-values in the result
  postprocess();

  return stat;
}

ScaLP::Result ScaLP::Solver::getResult()
{
  return result;
}

void ScaLP::Solver::reset()
{
  modelChanged=true;
  if(back!=nullptr) back->reset();
  objective=ScaLP::Objective();
  cons.clear();
  result=ScaLP::Result();
  warmStartValues=ScaLP::Result();
  warmStart=false;
}

// x*d
ScaLP::Term ScaLP::operator*(const ScaLP::Variable& v,double coeff)
{

  return ScaLP::Term(v,coeff);
}
ScaLP::Term ScaLP::operator*(ScaLP::Variable&& v,double coeff)
{
  return ScaLP::Term(v,coeff);
}
// d*x
ScaLP::Term ScaLP::operator*(double coeff, const ScaLP::Variable& v)
{
  return v*coeff;
}
ScaLP::Term ScaLP::operator*(double coeff, ScaLP::Variable&& v)
{
  return v*coeff;
}

// d(ax+by) = adx+bdy
ScaLP::Term ScaLP::operator*(double coeff, const ScaLP::Term& t)
{
  ScaLP::Term n = t;
  n.constant*=coeff;
  for(auto &p:n.sum)
  {
    p.second*=coeff;
  }
  return n;
}
ScaLP::Term ScaLP::operator*(double coeff, ScaLP::Term&& n)
{
  n.constant*=coeff;
  for(auto &p:n.sum)
  {
    p.second*=coeff;
  }
  return n;
}
// (xa+yb)d = xad+ybd
ScaLP::Term ScaLP::operator*(const ScaLP::Term& t, double coeff)
{
  return coeff*t;
}
ScaLP::Term ScaLP::operator*(ScaLP::Term&& t, double coeff)
{
  return coeff*t;
}

// insert a {key,value}-pair to the map, adjust already present values with the function f.
// returns an iterator to the possibly eliminated variable
static  std::map<ScaLP::Variable,double>::iterator adjust(std::map<ScaLP::Variable,double> &m,const ScaLP::Variable& k,double v,const std::function<double(double,double)>& f)
{
  if(v==0) return m.end();
  auto it = m.find(k);
  if(it==m.end())
  {
    m.emplace(k,v);
    return it;
  }
  else
  {
    it->second = f(it->second,v);
    if(it->second==0) return it;
    else return m.end();
  }
}

ScaLP::Term ScaLP::operator+(const ScaLP::Term& tl,const ScaLP::Term& tr)
{
  Term n = tl;
  n.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    auto it = adjust(n.sum,p.first,p.second,plus);
    if(it!=n.sum.end()) n.sum.erase(it);
  }
  return n;
}
ScaLP::Term ScaLP::operator+(const ScaLP::Term& tl, ScaLP::Term&& n)
{
  n.constant+=tl.constant;
  for(auto &p:tl.sum)
  {
    auto it = adjust(n.sum,p.first,p.second,plus);
    if(it!=n.sum.end()) n.sum.erase(it);
  }
  return n;
}
ScaLP::Term ScaLP::operator+(ScaLP::Term&& tl, const ScaLP::Term& tr)
{
  tl.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    auto it = adjust(tl.sum,p.first,p.second,plus);
    if(it!=tl.sum.end()) tl.sum.erase(it);
  }
  return tl;
}
ScaLP::Term ScaLP::operator+(ScaLP::Term&& tl, ScaLP::Term&& tr)
{
  tl.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    auto it = adjust(tl.sum,p.first,p.second,plus);
    if(it!=tl.sum.end()) tl.sum.erase(it);
  }
  return tl;
}

ScaLP::Term& ScaLP::operator+=(ScaLP::Term &tl, const ScaLP::Term& tr)
{
  tl.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    auto it = adjust(tl.sum,p.first,p.second,plus);
    if(it!=tl.sum.end()) tl.sum.erase(it);
  }
  return tl;
}

ScaLP::Term ScaLP::operator-(const ScaLP::Term& t)
{
  ScaLP::Term n = t;
  n.constant*= -1;
  for(auto& p:n.sum) p.second*=-1;
  return n;
}
ScaLP::Term ScaLP::operator-(ScaLP::Term&& n)
{
  n.constant*= -1;
  for(auto& p:n.sum) p.second*=-1;
  return n;
}

ScaLP::Term ScaLP::operator-(const ScaLP::Variable& v)
{
  return (-1)*v;
}

ScaLP::Term ScaLP::operator-(const ScaLP::Term& tl, const ScaLP::Term& tr)
{
  Term n = tl;
  n.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    auto it = adjust(n.sum,p.first,-p.second,plus);
    if(it!=n.sum.end()) n.sum.erase(it);
  }
  return n;
}

ScaLP::Term& ScaLP::operator-=(ScaLP::Term& tl, const ScaLP::Term& tr)
{
  tl.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    auto it = adjust(tl.sum,p.first,-p.second,plus);
    if(it!=tl.sum.end()) tl.sum.erase(it);
  }
  return tl;
}

ScaLP::Term& ScaLP::operator*=(ScaLP::Term& tl,double d)
{
  tl.constant*=d;
  for(auto &p:tl.sum)
  {
    p.second*=d;
  }
  return tl;
}

bool ScaLP::Solver::setRelativeMIPGap(double d)
{
  if(d>=0)
  {
    relMIPGap=d;
    return true;
  }
  else
  {
    return false; // out of range
  }
}

bool ScaLP::Solver::setAbsoluteMIPGap(double d)
{
  if(d>=0)
  {
    absMIPGap=d;
    return true;
  }
  else
  {
    return false; // out of range
  }
}

void ScaLP::Solver::resetMIPGap()
{
  absMIPGap=-1;
  relMIPGap=-1;
}

bool ScaLP::Solver::featureSupported(ScaLP::Feature f) const
{
  if(back!=nullptr)
  {
    return back->featureSupported(f);
  }
  return false;
}

bool ScaLP::Solver::isFeasible(const ScaLP::Result& sol)
{
  for(auto&c:cons)
  {
    if(not c.isFeasible(sol)) return false;
  }
  return true;
}


#define ScaLP_RELATION_OPERATOR(A,B,C,D) \
  ScaLP::Constraint ScaLP::operator A(C l,D r) \
  { \
    return ScaLP::Constraint(l,ScaLP::relation::B,r); \
  }
#define ScaLP_RELATION_OPERATORVL(A,B,C,D) \
  ScaLP::Constraint ScaLP::operator A(C l,D r) \
  { \
    return ScaLP::Constraint(l,ScaLP::relation::B,r); \
  }
#define ScaLP_RELATION_OPERATORVR(A,B,C,D) \
  ScaLP::Constraint ScaLP::operator A(C l,D r) \
  { \
    return ScaLP::Constraint(l,ScaLP::relation::B,r); \
  }


ScaLP_RELATION_OPERATORVL(<=,LESS_EQ_THAN, const ScaLP::Variable&, double)
ScaLP_RELATION_OPERATORVL(>=,MORE_EQ_THAN, const ScaLP::Variable&, double)
ScaLP_RELATION_OPERATORVL(==,EQUAL       , const ScaLP::Variable&, double)
ScaLP_RELATION_OPERATORVR(<=,LESS_EQ_THAN, double, const ScaLP::Variable&)
ScaLP_RELATION_OPERATORVR(>=,MORE_EQ_THAN, double, const ScaLP::Variable&)
ScaLP_RELATION_OPERATORVR(==,EQUAL       , double, const ScaLP::Variable&)

ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, const ScaLP::Term&, double)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, const ScaLP::Term&, double)
ScaLP_RELATION_OPERATOR(==,EQUAL       , const ScaLP::Term&, double)
ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ScaLP::Term&&, double)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ScaLP::Term&&, double)
ScaLP_RELATION_OPERATOR(==,EQUAL       , ScaLP::Term&&, double)
ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, const ScaLP::Term&)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, const ScaLP::Term&)
ScaLP_RELATION_OPERATOR(==,EQUAL       , double, const ScaLP::Term&)
ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, ScaLP::Term&&)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, ScaLP::Term&&)
ScaLP_RELATION_OPERATOR(==,EQUAL       , double, ScaLP::Term&&)

ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, const ScaLP::Constraint&, double)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, const ScaLP::Constraint&, double)
ScaLP_RELATION_OPERATOR(==,EQUAL       , const ScaLP::Constraint&, double)
ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ScaLP::Constraint&&, double)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ScaLP::Constraint&&, double)
ScaLP_RELATION_OPERATOR(==,EQUAL       , ScaLP::Constraint&&, double)

ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, const ScaLP::Constraint&)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, const ScaLP::Constraint&)
ScaLP_RELATION_OPERATOR(==,EQUAL       , double, const ScaLP::Constraint&)
ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, ScaLP::Constraint&&)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, ScaLP::Constraint&&)
ScaLP_RELATION_OPERATOR(==,EQUAL       , double, ScaLP::Constraint&&)

#define WRONG_VARIABLE_EXCEPTION(V) ScaLP::Exception("The given Variable \""+V->getName()+"\" is not an Integer-Variable")
static bool isIntegralType(const ScaLP::Variable& v)
{
  auto t = v->getType();
  return t==ScaLP::VariableType::INTEGER or t==ScaLP::VariableType::BINARY;
}

ScaLP::Constraint ScaLP::operator<(const ScaLP::Variable& tl,double tr)
{
  if(isIntegralType(tl))
    return ScaLP::Constraint(tl,ScaLP::relation::LESS_EQ_THAN,tr-1);
  else
    throw WRONG_VARIABLE_EXCEPTION(tl);
}
ScaLP::Constraint ScaLP::operator<(double tl,const ScaLP::Variable& tr)
{
  if(isIntegralType(tr))
    return ScaLP::Constraint(tl+1,ScaLP::relation::LESS_EQ_THAN,tr);
  else
    throw WRONG_VARIABLE_EXCEPTION(tr);
}
ScaLP::Constraint ScaLP::operator<(ScaLP::Term&& tl,double tr)
{
  for(const auto& v:tl.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl,ScaLP::relation::LESS_EQ_THAN,tr-1);
}
ScaLP::Constraint ScaLP::operator<(const ScaLP::Term& tl,double tr)
{
  for(const auto& v:tl.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl,ScaLP::relation::LESS_EQ_THAN,tr-1);
}
ScaLP::Constraint ScaLP::operator<(double tl,ScaLP::Term&& tr)
{
  for(const auto& v:tr.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl+1,ScaLP::relation::LESS_EQ_THAN,tr);
}
ScaLP::Constraint ScaLP::operator<(double tl,const ScaLP::Term& tr)
{
  for(const auto& v:tr.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl+1,ScaLP::relation::LESS_EQ_THAN,tr);
}

ScaLP::Constraint ScaLP::operator>(const ScaLP::Variable& tl,double tr)
{
  if(isIntegralType(tl))
    return ScaLP::Constraint(tl,ScaLP::relation::LESS_EQ_THAN,tr+1);
  else
    throw WRONG_VARIABLE_EXCEPTION(tl);
}
ScaLP::Constraint ScaLP::operator>(double tl,const ScaLP::Variable& tr)
{
  if(isIntegralType(tr))
    return ScaLP::Constraint(tl-1,ScaLP::relation::LESS_EQ_THAN,tr);
  else
    throw WRONG_VARIABLE_EXCEPTION(tr);
}
ScaLP::Constraint ScaLP::operator>(ScaLP::Term&& tl,double tr)
{
  for(const auto& v:tl.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl,ScaLP::relation::LESS_EQ_THAN,tr+1);
}
ScaLP::Constraint ScaLP::operator>(const ScaLP::Term& tl,double tr)
{
  for(const auto& v:tl.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl,ScaLP::relation::LESS_EQ_THAN,tr+1);
}
ScaLP::Constraint ScaLP::operator>(double tl,ScaLP::Term&& tr)
{
  for(const auto& v:tr.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl-1,ScaLP::relation::LESS_EQ_THAN,tr);
}
ScaLP::Constraint ScaLP::operator>(double tl,const ScaLP::Term& tr)
{
  for(const auto& v:tr.sum)
  {
    if(not isIntegralType(v.first))
      throw WRONG_VARIABLE_EXCEPTION(v.first);
  }
  return ScaLP::Constraint(tl-1,ScaLP::relation::LESS_EQ_THAN,tr);
}

ScaLP::Solver& ScaLP::operator<<(Solver &s,const Objective& o)
{
  s.setObjective(o);
  return s;
}
ScaLP::Solver& ScaLP::operator<<(Solver &s,Constraint& o)
{
  s.addConstraint(o);
  return s;
}
ScaLP::Solver& ScaLP::operator<<(Solver &s,Constraint&& o)
{
  s.addConstraint(o);
  return s;
}

ScaLP::Constraint ScaLP::operator>>=(const ScaLP::Constraint& i,const ScaLP::Constraint& c)
{
  return ScaLP::Constraint(i,c);
}
