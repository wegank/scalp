
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

void ScaLP::Solver::setObjective(Objective o)
{
  this->objective=o;

  // this should throw an exception if the Objective rises a name-collision
  extractVariables(cons,objective);
}

void ScaLP::Solver::addConstraint(Constraint& b)
{
  this->cons.push_back(b);
}
void ScaLP::Solver::addConstraint(Constraint&& b)
{
  this->cons.push_back(b);
}

static std::string showTermLP(ScaLP::Term t)
{
  std::stringstream s;

  // only constant
  if(t.isConstant())
  {
    s << std::to_string(t.constant);
    return s.str();
  }

  bool first=true; // first iteration
  for(auto &p:t.sum)
  {
    // eliminated Variable
    if(p.second==0) continue;

    // show coefficient?
    if(p.second!=1 && p.second!=-1)
    {
      if(!first)
      {
        if(p.second<0)
          s << " - " << -p.second;
        else
          s << " + " << p.second;
      }
      else
      {
        if(p.second<0)
          s << "-" << -p.second << " ";
        else
          s << p.second << " ";
      }
    }

    // signed variable?
    if(p.second==-1)
    {
      if(first)
        s << "-";
      else
        s << " -";
    }

    if(p.second==1 && !first)
      s << " +";
      
    if(!first) s << " ";

    s << p.first->getName();
    first=false;
  }

  // constant part
  if(t.constant!=0)
  {
    if(t.constant<0)
      s << " - " << -t.constant;
    else
      s << " + " << t.constant;
  }

  return s.str();
}

static std::string showObjectiveLP(ScaLP::Objective o)
{
  std::stringstream s;
  if(o.getType()==ScaLP::Objective::type::MAXIMIZE)
  {
    s<<"MAXIMIZE";
  }
  else
  {
    s<<"MINIMIZE";
  }
  s << "\n  " << showTermLP(o.getTerm()) << "\n";

  return s.str();
}

static std::string showConstraint2LP(ScaLP::Term lhs,ScaLP::relation rel, ScaLP::Term rhs)
{
  std::stringstream s;
  s << showTermLP(lhs) << " " << ScaLP::Constraint::showRelation(rel) << " " << showTermLP(rhs);
  return s.str();
}

static std::string showConstraint3LP(const ScaLP::Constraint& c)
{
  std::stringstream s;
  s << showTermLP(c.lbound) << " " << ScaLP::Constraint::showRelation(c.lrel) << " "
    << showTermLP(c.term)   << " " << ScaLP::Constraint::showRelation(c.rrel) << " "
    << showTermLP(c.ubound);
  return s.str();
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

static std::string showConstraintLP(ScaLP::Constraint c)
{
  normalizeConstraint(c);

  std::string prefix="";

  if(c.name!="")
  {
    prefix = c.name+": ";
  }

  if(c.indicator!=nullptr)
  {
    prefix= prefix+showConstraint2LP(c.indicator->term,c.indicator->lrel,c.indicator->lbound)+ " -> ";
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

static std::string variableTypesLP(ScaLP::VariableSet &vs)
{
  std::string binary="BINARY\n";
  std::string general="GENERAL\n";
  for(const ScaLP::Variable v:vs)
  {
    if(v->getType()==ScaLP::VariableType::INTEGER)
      general+="  "+v->getName()+"\n";
    else if(v->getType()==ScaLP::VariableType::BINARY)
      binary+="  "+v->getName()+"\n";

  }
  return binary+general;
}

static std::string boundsLP(ScaLP::VariableSet &vs)
{
  std::stringstream s;
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
      s << name << " FREE";
    }

    // x <= b
    else if(lb==0 and ub!=ScaLP::INF())
    {
      s << name << " <= " << ub;
    }

    // a <= x
    else if(lb!=0 && ub==ScaLP::INF())
    {
      s << lb << " <= " << name;
    }

    // a <= x <= b
    else
    {
      s << lb << " <= " << name << " <= " << ub;
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

std::string ScaLP::Solver::showLP() const
{
  std::stringstream s;

  s << showObjectiveLP(objective);

  s << "SUBJECT TO\n";
  for(auto &c:cons)
  {
    s << "  " << showConstraintLP(c) << "\n";
  }
  
  ScaLP::VariableSet vs = extractVariables(cons,objective);

  s << "BOUNDS\n";
  s << boundsLP(vs);

  s << variableTypesLP(vs);

  s << "END\n";

  return s.str();

}

void ScaLP::Solver::writeLP(std::string file) const
{
  std::ofstream(file) << showLP();
}

void ScaLP::Solver::prepare()
{
  // set the used threads
  if(threads>0) back->setThreads(threads);

  // set the verbosity of the backend
  back->setConsoleOutput(!quiet);

  if(timeout>0) back->setTimeout(timeout);

  if(absMIPGap>=0) back->setAbsoluteMIPGap(absMIPGap);
  if(relMIPGap>=0) back->setRelativeMIPGap(relMIPGap);

  // use presolve?
  back->presolve(presolve);
}

void ScaLP::Solver::construct()
{
  // Add the Variables
  // Only add the used Variables.
  back->addVariables(extractVariables(cons,objective));

  // Add Objective
  back->setObjective(objective);

  // Add Constraints
  back->addConstraints(cons);
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

ScaLP::status ScaLP::Solver::solve()
{
  if(not resultCacheDir.empty())
  {
    auto hash = ScaLP::hashFNV(this->showLP());
    auto s =  extractVariables(cons,objective);
    std::vector<ScaLP::Variable> v{s.begin(),s.end()};
    if(ScaLP::hasOptimalSolution(resultCacheDir,hash))
    {
      this->result = ScaLP::getOptimalSolution(resultCacheDir,hash,v);
      return ScaLP::status::OPTIMAL;
    }
    else
    {
      auto stat = newSolve();
      if(stat==ScaLP::status::OPTIMAL)
      {
        ScaLP::writeOptimalSolution(resultCacheDir,hash,this->result,*this);
      }
      return stat;
    }
  }
  else
  {
    return newSolve();
  }
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
  if(back!=nullptr) back->reset();
  objective=ScaLP::Objective();
  cons.clear();
  result=ScaLP::Result();
}

ScaLP::VariableSet ScaLP::Solver::extractVariables(const std::list<Constraint> &cs,const Objective &o) const
{
  ScaLP::VariableSet vs;

  ScaLP::VariableSet ovs = o.getTerm().extractVariables();
  vs.insert(ovs.begin(),ovs.end());

  for(auto& c:cs)
  {
    ScaLP::VariableSet vvs = c.extractVariables();
    vs.insert(vvs.begin(),vvs.end());
  }

  return vs;
}

// x*d
ScaLP::Term ScaLP::operator*(ScaLP::Variable v,double coeff)
{
  ScaLP::Term t;
  t.add(v,coeff);
  return t;
}
// d*x
ScaLP::Term ScaLP::operator*(double coeff, ScaLP::Variable v)
{
  return v*coeff;
}

// d(ax+by) = adx+bdy
ScaLP::Term ScaLP::operator*(double coeff, ScaLP::Term t)
{
  ScaLP::Term n = t;
  n.constant*=coeff;
  for(auto &p:n.sum)
  {
    p.second*=coeff;
  }
  return n;
}
// (xa+yb)d = xad+ybd
ScaLP::Term ScaLP::operator*(ScaLP::Term t, double coeff)
{
  return coeff*t;
}

// insert a {key,value}-pair to the map, adjust already present values with the function f.
static void adjust(std::map<ScaLP::Variable,double> &m,ScaLP::Variable k,double v,std::function<double(double,double)> f)
{
  auto it = m.find(k);
  if(it==m.end())
  {
    m.emplace(k,v);
  }
  else
  {
    it->second = f(it->second,v);
  }
}

ScaLP::Term ScaLP::operator+(ScaLP::Term tl,ScaLP::Term tr)
{
  Term n = tl;
  n.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(n.sum,p.first,p.second,plus);
  }
  return n;
}

ScaLP::Term& ScaLP::operator+=(ScaLP::Term &tl,ScaLP::Term tr)
{
  tl.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(tl.sum,p.first,p.second,plus);
  }
  return tl;
}

ScaLP::Term ScaLP::operator-(ScaLP::Term t)
{
  ScaLP::Term n = t;
  n.constant*= -1;
  for(auto& p:n.sum) p.second*=-1;
  return n;
}

ScaLP::Term ScaLP::operator-(ScaLP::Variable v)
{
  return (-1)*v;
}

ScaLP::Term ScaLP::operator-(ScaLP::Term tl, ScaLP::Term tr)
{
  Term n = tl;
  n.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(n.sum,p.first,-p.second,plus);
  }
  return n;
}

ScaLP::Term& ScaLP::operator-=(ScaLP::Term &tl, ScaLP::Term tr)
{
  tl.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(tl.sum,p.first,-p.second,plus);
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

bool ScaLP::Solver::featureSupported(ScaLP::Feature f)
{
  if(back!=nullptr)
  {
    return back->featureSupported(f);
  }
  return false;
}


#define ScaLP_RELATION_OPERATOR(A,B,C,D) \
  ScaLP::Constraint ScaLP::operator A(C l,D r) \
  { \
    return ScaLP::Constraint(l,ScaLP::relation::B,r); \
  }
#define ScaLP_RELATION_OPERATORVL(A,B,C,D) \
  ScaLP::Constraint ScaLP::operator A(C l,D r) \
  { \
    ScaLP::Term t = l;\
    return ScaLP::Constraint(t,ScaLP::relation::B,r); \
  }
#define ScaLP_RELATION_OPERATORVR(A,B,C,D) \
  ScaLP::Constraint ScaLP::operator A(C l,D r) \
  { \
    ScaLP::Term t = r;\
    return ScaLP::Constraint(l,ScaLP::relation::B,t); \
  }


ScaLP_RELATION_OPERATORVL(<=,LESS_EQ_THAN, ScaLP::Variable, double)
ScaLP_RELATION_OPERATORVL(>=,MORE_EQ_THAN, ScaLP::Variable, double)
ScaLP_RELATION_OPERATORVL(==,EQUAL       , ScaLP::Variable, double)
ScaLP_RELATION_OPERATORVR(<=,LESS_EQ_THAN, double, ScaLP::Variable)
ScaLP_RELATION_OPERATORVR(>=,MORE_EQ_THAN, double, ScaLP::Variable)
ScaLP_RELATION_OPERATORVR(==,EQUAL       , double, ScaLP::Variable)

ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ScaLP::Term, double)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ScaLP::Term, double)
ScaLP_RELATION_OPERATOR(==,EQUAL       , ScaLP::Term, double)
ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, ScaLP::Term)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, ScaLP::Term)
ScaLP_RELATION_OPERATOR(==,EQUAL       , double, ScaLP::Term)

ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ScaLP::Constraint, double)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ScaLP::Constraint, double)
ScaLP_RELATION_OPERATOR(==,EQUAL       , ScaLP::Constraint, double)

ScaLP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, ScaLP::Constraint)
ScaLP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, ScaLP::Constraint)
ScaLP_RELATION_OPERATOR(==,EQUAL       , double, ScaLP::Constraint)

ScaLP::Solver& ScaLP::operator<<(Solver &s,Objective o)
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

ScaLP::Constraint ScaLP::operator>>=(ScaLP::Constraint i,ScaLP::Constraint c)
{
  return ScaLP::Constraint(i,c);
}
