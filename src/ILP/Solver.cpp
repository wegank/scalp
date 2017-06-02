
#include <limits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <functional> // std::puls, std::minus
#include <ctime>

#include <ILP/Exception.h>
#include <ILP/Solver.h>
#include <ILP/Result.h>

using namespace ILP;

double ILP::INF()
{
  return std::numeric_limits<double>::infinity();
}

ILP::Solver::Solver(ILP::SolverBackend *b)
  :back(b)
{
}

ILP::Solver::~Solver()
{
  delete back;
}

void Solver::setBackend(SolverBackend *b)
{
  this->back=b;
}

void Solver::setObjective(Objective o)
{
  this->objective=o;

  // this should throw an exception if the Objective rises a name-collision
  extractVariables(cons,objective);
}

void Solver::addConstraint(Constraint& b)
{
  this->cons.push_back(b);
}
void Solver::addConstraint(Constraint&& b)
{
  this->cons.push_back(b);
}

static std::string showTermLP(ILP::Term t)
{
  std::stringstream s;

  // only constant
  if(t.isConstant())
  {
    s << t.constant;
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

    s << p.first->name;
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

static std::string showObjectiveLP(ILP::Objective o)
{
  std::stringstream s;
  if(o.usedType==ILP::Objective::type::MAXIMIZE)
  {
    s<<"MAXIMIZE";
  }
  else
  {
    s<<"MINIMIZE";
  }
  s << "\n  " << showTermLP(o.usedTerm) << "\n";

  return s.str();
}

static std::string showConstraint2LP(ILP::Term lhs,ILP::relation rel, ILP::Term rhs)
{
  std::stringstream s;
  s << showTermLP(lhs) << " " << ILP::Constraint::showRelation(rel) << " " << showTermLP(rhs);
  return s.str();
}
static std::string showConstraint3LP(const ILP::Constraint& c)
{
  std::stringstream s;
  s << showTermLP(c.lbound) << " " << ILP::Constraint::showRelation(c.lrel) << " "
    << showTermLP(c.term)   << " " << ILP::Constraint::showRelation(c.rrel) << " "
    << showTermLP(c.ubound);
  return s.str();
}
static std::string showConstraintLP(ILP::Constraint c)
{
  switch(c.ctype)
  {
    case ILP::Constraint::type::C2L:
      return showConstraint2LP(c.lbound,c.lrel,c.term);
    case ILP::Constraint::type::C2R:
      return showConstraint2LP(c.term,c.rrel,c.ubound);
    case ILP::Constraint::type::CEQ:
      return showConstraint2LP(c.lbound,c.lrel,c.term);
    case ILP::Constraint::type::C3:
      return showConstraint3LP(c);
  }
  return "";
}

static std::string variableTypesLP(ILP::VariableSet &vs)
{
  std::string binary="BINARY\n";
  std::string general="GENERAL\n";
  for(const ILP::Variable v:vs)
  {
    if(v->usedType==ILP::VariableType::INTEGER)
      general+="  "+v->name+"\n";
    else if(v->usedType==ILP::VariableType::BINARY)
      binary+="  "+v->name+"\n";

  }
  return binary+general;
}

static std::string boundsLP(ILP::VariableSet &vs)
{
  std::stringstream s;
  for(const auto &v:vs)
  {
    // default
    if(v->lowerRange==0 && v->upperRange==ILP::INF())
    {
      continue;
    }

    // indentation
    s << "  ";

    // free
    if(v->lowerRange==-ILP::INF() && v->upperRange==ILP::INF())
    {
      s << v->name << " FREE";
    }

    // x <= b
    else if(v->lowerRange==0 && v->upperRange!=ILP::INF())
    {
      s << v->name << " <= " << v->upperRange;
    }

    // a <= x
    else if(v->lowerRange!=0 && v->upperRange==ILP::INF())
    {
      s << v->lowerRange << " <= " << v->name;
    }

    // a <= x <= b
    //if(v->lowerRange!=0 && v->upperRange!=ILP::INF())
    else
    {
      s << v->lowerRange << " <= " << v->name << " <= " << v->upperRange;
    }

    // end of entry
    s << "\n";

  }
  return s.str();
}

std::string ILP::Solver::getBackendName() const
{
  return back->name;
}

std::string ILP::Solver::showLP() const
{
  std::stringstream s;

  s << showObjectiveLP(objective);

  s << "SUBJECT TO\n";
  for(auto &c:cons)
  {
    s << "  " << showConstraintLP(c) << "\n";
  }
  
  ILP::VariableSet vs = extractVariables(cons,objective);

  s << "BOUNDS\n";
  s << boundsLP(vs);

  s << variableTypesLP(vs);

  s << "END\n";

  return s.str();

}

void ILP::Solver::writeLP(std::string file) const
{
  std::ofstream(file) << showLP();
}

ILP::status Solver::solve()
{
  double preparationTime=0;
  double constructionTime=0;
  double solvingTime=0;

  auto timer = std::clock();

  // reset the backend
  back->reset();

  // set the used threads
  if(threads>0) back->setThreads(threads);

  // set the verbosity of the backend
  back->setConsoleOutput(!quiet);

  if(timeout>0) back->setTimeout(timeout);

  if(absMIPGap>=0) back->setAbsoluteMIPGap(absMIPGap);
  if(relMIPGap>=0) back->setRelativeMIPGap(relMIPGap);

  preparationTime = (double(std::clock()-timer))/CLOCKS_PER_SEC;
  timer = std::clock();

  // Add the Variables
  // Only add the used Variables.
  back->addVariables(extractVariables(cons,objective));

  // Add Objective
  back->setObjective(objective);

  // Add Constraints
  back->addConstraints(cons);

  constructionTime = (double(std::clock()-timer))/CLOCKS_PER_SEC;
  timer = std::clock();

  // use presolve?
  back->presolve(presolve);

  // Solve
  ILP::status stat = back->solve();

  solvingTime = (double(std::clock()-timer))/CLOCKS_PER_SEC;

  back->res.preparationTime=preparationTime;
  back->res.constructionTime=constructionTime;
  back->res.solvingTime=solvingTime;

  return stat;
}

ILP::Result Solver::getResult()
{
  return this->back->res;
}

void Solver::reset()
{
  if(back!=nullptr) back->reset();
  objective= ILP::Objective();
  cons.clear();
  result=ILP::Result();
}

ILP::VariableSet Solver::extractVariables(std::list<Constraint> cs,Objective o) const
{
  ILP::VariableSet vs;

  ILP::VariableSet ovs = o.usedTerm.extractVariables();
  vs.insert(ovs.begin(),ovs.end());

  for(ILP::Constraint& c:cs)
  {
    ILP::VariableSet vvs = c.extractVariables();
    vs.insert(vvs.begin(),vvs.end());
  }

  return vs;
}

// x*d
ILP::Term ILP::operator*(ILP::Variable v,double coeff)
{
  ILP::Term t;
  t.add(v,coeff);
  return t;
}
// d*x
ILP::Term ILP::operator*(double coeff, ILP::Variable v)
{
  return v*coeff;
}

// d(ax+by) = adx+bdy
ILP::Term ILP::operator*(double coeff, ILP::Term t)
{
  ILP::Term n = t;
  n.constant*=coeff;
  for(auto &p:n.sum)
  {
    p.second*=coeff;
  }
  return n;
}
// (xa+yb)d = xad+ybd
ILP::Term ILP::operator*(ILP::Term t, double coeff)
{
  return coeff*t;
}

// insert a {key,value}-pair to the map, adjust already present values with the function f.
static void adjust(std::map<ILP::Variable,double> &m,ILP::Variable k,double v,std::function<double(double,double)> f)
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

ILP::Term ILP::operator+(ILP::Term tl,ILP::Term tr)
{
  Term n = tl;
  n.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(n.sum,p.first,p.second,std::plus<double>());
  }
  return n;
}

ILP::Term& ILP::operator+=(ILP::Term &tl,ILP::Term tr)
{
  tl.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(tl.sum,p.first,p.second,std::plus<double>());
  }
  return tl;
}

ILP::Term ILP::operator-(ILP::Term t)
{
  ILP::Term n = t;
  n.constant*= -1;
  for(auto& p:n.sum) p.second*=-1;
  return n;
}

ILP::Term ILP::operator-(ILP::Variable v)
{
  return (-1)*v;
}

ILP::Term ILP::operator-(ILP::Term tl, ILP::Term tr)
{
  Term n = tl;
  n.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(n.sum,p.first,-p.second,std::plus<double>());
  }
  return n;
}

ILP::Term& ILP::operator-=(ILP::Term &tl, ILP::Term tr)
{
  tl.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(tl.sum,p.first,-p.second,std::plus<double>());
  }
  return tl;
}

ILP::Term& ILP::operator*=(ILP::Term& tl,double d)
{
  tl.constant*=d;
  for(auto &p:tl.sum)
  {
    p.second*=d;
  }
  return tl;
}

bool ILP::Solver::setRelativeMIPGap(double d)
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

bool ILP::Solver::setAbsoluteMIPGap(double d)
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

void ILP::Solver::resetMIPGap()
{
  absMIPGap=-1;
  relMIPGap=-1;
}


#define ILP_RELATION_OPERATOR(A,B,C,D) \
  ILP::Constraint ILP::operator A(C l,D r) \
  { \
    return ILP::Constraint(l,ILP::relation::B,r); \
  }

ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, ILP::Term)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, ILP::Term)
ILP_RELATION_OPERATOR(==,EQUAL       , double, ILP::Term)
ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ILP::Term, double)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ILP::Term, double)
ILP_RELATION_OPERATOR(==,EQUAL       , ILP::Term, double)

ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ILP::Constraint, double)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ILP::Constraint, double)
ILP_RELATION_OPERATOR(==,EQUAL       , ILP::Constraint, double)

ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, double, ILP::Constraint)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, double, ILP::Constraint)
ILP_RELATION_OPERATOR(==,EQUAL       , double, ILP::Constraint)

Solver& ILP::operator<<(Solver &s,Objective o)
{
  s.setObjective(o);
  return s;
}
Solver& ILP::operator<<(Solver &s,Constraint& o)
{
  s.addConstraint(o);
  return s;
}
Solver& ILP::operator<<(Solver &s,Constraint&& o)
{
  s.addConstraint(o);
  return s;
}
