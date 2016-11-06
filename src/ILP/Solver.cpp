
#include <limits>
#include <iostream>
#include <sstream>
#include <fstream>
#include <functional> // std::puls, std::minus

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
}

void Solver::addConstraint(Constraint b)
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

static std::string showConstraintLP(ILP::Constraint2 c)
{
  std::stringstream s;
  s << showTermLP(c.lhs) << " " << ILP::Constraint::showRelation(c.usedRelation) << " " << showTermLP(c.rhs);
  return s.str();
}
static std::string showConstraintLP(ILP::Constraint3 c)
{
  std::stringstream s;
  s << showTermLP(c.lbound) << " " << ILP::Constraint::showRelation(c.lrel) << " "
    << showTermLP(c.term)   << " " << ILP::Constraint::showRelation(c.rrel) << " "
    << showTermLP(c.ubound);
  return s.str();
}
static std::string showConstraintLP(ILP::Constraint c)
{
  if(c.usedType == ILP::Constraint::type::Constraint_2)
    return showConstraintLP(c.c2);
  else if(c.usedType == ILP::Constraint::type::Constraint_3)
    return showConstraintLP(c.c3);
  else
    return "";
}

static std::string variableTypesLP(ILP::VariableSet &vs)
{
  std::string binary="BINARY\n";
  std::string general="GENARAL\n";
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

std::string ILP::Solver::showLP()
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

void ILP::Solver::writeLP(std::string file)
{
  std::ofstream(file) << showLP();
}

ILP::status Solver::solve()
{
  // reset the backend
  back->reset();

  // set the verbosity of the backend
  back->setConsoleOutput(!quiet);

  // Add the Variables
  // Only add the used Variables.
  ILP::VariableSet vs = extractVariables(cons,objective);

  back->addVariables(vs);

  // Add Objective
  back->setObjective(objective);

  // Add Constraints
  back->addConstraints(cons);

  // Call the optional preSolve-Function
  if(back->preSolve) back->preSolve();

  // Solve
  ILP::status stat = back->solve();

  // Call the optional postSolve-Function
  if(back->postSolve) back->postSolve();

  return stat;
}

ILP::Result Solver::getResult()
{
  return this->back->res;
}

ILP::VariableSet Solver::extractVariables(std::list<Constraint> cs,Objective o)
{
  ILP::VariableSet vs;

  ILP::VariableSet ovs = o.usedTerm.extractVariables();
  vs.insert(ovs.begin(),ovs.end());

  for(ILP::Constraint c:cs)
  {
    ILP::VariableSet vvs = c.extractVariables();
    vs.insert(vvs.begin(),vvs.end());
  }

  return vs;
}

// x*d
ILP::Term operator*(ILP::Variable v,double coeff)
{
  ILP::Term t;
  t.add(v,coeff);
  return t;
}
// d*x
ILP::Term operator*(double coeff, ILP::Variable v)
{
  return v*coeff;
}

// d(ax+by) = adx+bdy
ILP::Term operator*(double coeff, ILP::Term t)
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
ILP::Term operator*(ILP::Term t, double coeff)
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

ILP::Term operator+(ILP::Term tl,ILP::Term tr)
{
  Term n = tl;
  n.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(n.sum,p.first,p.second,std::plus<double>());
  }
  return n;
}

ILP::Term& operator+=(ILP::Term &tl,ILP::Term tr)
{
  tl.constant+=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(tl.sum,p.first,p.second,std::plus<double>());
  }
  return tl;
}

ILP::Term operator-(ILP::Term tl, ILP::Term tr)
{
  Term n = tl;
  n.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(n.sum,p.first,p.second,std::minus<double>());
  }
  return n;
}

ILP::Term& operator-=(ILP::Term &tl, ILP::Term tr)
{
  tl.constant-=tr.constant;
  for(auto &p:tr.sum)
  {
    adjust(tl.sum,p.first,p.second,std::minus<double>());
  }
  return tl;
}

ILP::Term& operator*=(ILP::Term& tl,double d)
{
  tl.constant*=d;
  for(auto &p:tl.sum)
  {
    p.second*=d;
  }
  return tl;
}

ILP::Term operator-(ILP::Variable v)
{
  return (-1)*v;
}

#define ILP_RELATION_OPERATOR(A,B,C,D) \
  ILP::Constraint operator A(C l,D r) \
  { \
    ILP::Constraint c(l,ILP::relation::B,r); \
    return c; \
  }


ILP_RELATION_OPERATOR(< ,LESS_THAN   , ILP::Term, ILP::Term)
ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ILP::Term, ILP::Term)
ILP_RELATION_OPERATOR(> ,MORE_THAN   , ILP::Term, ILP::Term)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ILP::Term, ILP::Term)
ILP_RELATION_OPERATOR(==,EQUAL       , ILP::Term, ILP::Term)

ILP_RELATION_OPERATOR(< ,LESS_THAN   , ILP::Constraint, ILP::Term)
ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ILP::Constraint, ILP::Term)
ILP_RELATION_OPERATOR(> ,MORE_THAN   , ILP::Constraint, ILP::Term)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ILP::Constraint, ILP::Term)
ILP_RELATION_OPERATOR(==,EQUAL       , ILP::Constraint, ILP::Term)

ILP_RELATION_OPERATOR(< ,LESS_THAN   , ILP::Term, ILP::Constraint)
ILP_RELATION_OPERATOR(<=,LESS_EQ_THAN, ILP::Term, ILP::Constraint)
ILP_RELATION_OPERATOR(> ,MORE_THAN   , ILP::Term, ILP::Constraint)
ILP_RELATION_OPERATOR(>=,MORE_EQ_THAN, ILP::Term, ILP::Constraint)
ILP_RELATION_OPERATOR(==,EQUAL       , ILP::Term, ILP::Constraint)

Solver &operator<<(Solver &s,Objective o)
{
  s.setObjective(o);
  return s;
}
Solver &operator<<(Solver &s,Constraint o)
{
  s.addConstraint(o);
  return s;
}
