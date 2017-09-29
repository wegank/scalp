
#include <ILP/SolverBackend/SolverCPLEX.h>
#include <ILP/SolverBackend/SolverCPLEX_intern.h>

#include <ILP/Result.h>

#include <tuple>

ILP::SolverBackend* newSolverCPLEX()
{
  return new ILP::SolverCPLEX();
}

ILP::SolverBackend* ILP::newSolverCPLEX()
{
  return ::newSolverCPLEX();
}

ILP::SolverCPLEX::SolverCPLEX()
  :env(IloEnv()), model(IloModel(env))
{
  name="CPLEX";
}

static IloNumVar::Type mapVariableType(const ILP::Variable& v)
{
  switch(v->getType())
  {
    case ILP::VariableType::BINARY: return IloNumVar::Bool;
    case ILP::VariableType::INTEGER: return IloNumVar::Int;
    case ILP::VariableType::REAL: return IloNumVar::Float;
    default: throw ILP::Exception("Variable type not implemented yet.");
  }
  return ILOINT;
}

bool ILP::SolverCPLEX::addVariable(const ILP::Variable& v)
{
  bool r=false;
  try{
    auto p = variables.emplace(v,IloNumVar(env,v->getLowerBound(), v->getUpperBound(),mapVariableType(v),v->getName().c_str()));
    r=p.second;
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }
  return r;
}

IloExpr ILP::SolverCPLEX::mapTerm(const ILP::Term& t)
{
  IloExpr term= IloExpr(env)+t.constant;
  for(auto&p:t.sum)
  {
    term += variables.at(p.first)*p.second;
  }
  return term;
}

IloConstraint ILP::SolverCPLEX::createRange(double d, ILP::relation rel,const ILP::Term& t)
{
  switch(rel)
  {
    case ILP::relation::MORE_EQ_THAN:
      return d>=mapTerm(t);
    case ILP::relation::LESS_EQ_THAN:
      return d<=mapTerm(t);
    case ILP::relation::EQUAL:
      return d==mapTerm(t);
  }
}
IloConstraint ILP::SolverCPLEX::createRange(const ILP::Term& t, ILP::relation rel,double d)
{
  switch(rel)
  {
    case ILP::relation::MORE_EQ_THAN:
      return mapTerm(t)>=d;
    case ILP::relation::LESS_EQ_THAN:
      return mapTerm(t)<=d;
    case ILP::relation::EQUAL:
      return mapTerm(t)==d;
  }
}

IloConstraint ILP::SolverCPLEX::createConstraint3(const ILP::Constraint& c)
{
  if(c.lrel==ILP::relation::LESS_EQ_THAN && c.rrel==ILP::relation::LESS_EQ_THAN)
  { // a <= x <= b
    return c.lbound <= mapTerm(c.term) <= c.ubound;
  }
  else if(c.lrel==ILP::relation::MORE_EQ_THAN && c.rrel==ILP::relation::MORE_EQ_THAN)
  { // a >= x >= b
    return c.lbound >= mapTerm(c.term) >= c.ubound;
  }
}

IloConstraint ILP::SolverCPLEX::convertConstraint(const ILP::Constraint &c)
{
  IloConstraint constr;
  switch(c.ctype)
  {
    case ILP::Constraint::type::C3:
      constr = createConstraint3(c);
      break;
    case ILP::Constraint::type::C2L:
      constr = createRange(c.lbound,c.lrel,c.term);
      break;
    case ILP::Constraint::type::C2R:
      constr = createRange(c.term,c.rrel,c.ubound);
      break;
    case ILP::Constraint::type::CEQ:
      constr = createRange(c.lbound,c.lrel,c.term);
      break;
  }
  constr.setName(c.name.c_str());
  return constr;
}

bool ILP::SolverCPLEX::addConstraints(std::list<ILP::Constraint> cons)
{
  try
  {
    IloConstraintArray cc(env);
    for(const ILP::Constraint &c:cons)
    {
      if(c.indicator==nullptr)
      {
        cc.add(convertConstraint(c));
      }
      else
      {
        cc.add(IloIfThen(env,convertConstraint(*c.indicator),convertConstraint(c)));
      }
    }
    model.add(cc);
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }

  return true;
}

static ILP::status mapStatus(IloCplex::CplexStatus s)
{
  switch(s)
  {
    case IloCplex::CplexStatus::AbortTimeLim:    return ILP::status::TIMEOUT;
    case IloCplex::CplexStatus::Optimal:    return ILP::status::OPTIMAL;
    case IloCplex::CplexStatus::Unknown:    return ILP::status::UNKNOWN;
    case IloCplex::CplexStatus::Feasible:   return ILP::status::FEASIBLE;
    case IloCplex::CplexStatus::Infeasible: return ILP::status::INFEASIBLE;
    case IloCplex::CplexStatus::Unbounded:  return ILP::status::UNBOUND;
    case IloCplex::CplexStatus::InfOrUnbd:  return ILP::status::INFEASIBLE_OR_UNBOUND;
    default: return ILP::status::UNKNOWN;
  }
  return ILP::status::UNKNOWN;
}

bool ILP::SolverCPLEX::setObjective(ILP::Objective o)
{
  try
  {
    IloExpr obj(env);
    for(auto&p:o.getTerm().sum)
    {
      obj += variables.at(p.first) * p.second;
    }
    objectiveOffset=o.getTerm().constant;

    if(o.getType()==ILP::Objective::type::MINIMIZE)
    {
      model.add(IloMinimize(env, obj));
    }
    else
    {
      model.add(IloMaximize(env, obj));
    }
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }
  return true;
}

ILP::status ILP::SolverCPLEX::solve()
{
  ILP::status stat = ILP::status::ERROR;
  try
  {
    IloCplex cplex(model);

    // disable output
    if(not verbose) cplex.setOut(env.getNullStream());
    
    // set threads
    if(threads>0) cplex.setParam(IloCplex::Threads,threads);

    // set time limit
    if(timeout>0) cplex.setParam(IloCplex::TiLim,timeout);

    // simplify model if possible
    if(presolving)
    {
      cplex.setParam(IloCplex::PreInd,true);
    }
    else
    {
      cplex.setParam(IloCplex::PreInd,false);
      cplex.presolve(IloCplex::Algorithm::NoAlg);
    }

    // MIP-Gap
    if(this->relMIPGap>=0)
    {
      cplex.setParam(IloCplex::EpGap, this->relMIPGap);
    }
    if(this->absMIPGap>=0)
    {
      cplex.setParam(IloCplex::EpAGap, this->absMIPGap);
    }

    //cplex.exportModel("cplex.lp");

    if(cplex.solve())
    {
      // extract result
      for(auto&p:variables)
      {
        IloNum n = cplex.getValue(p.second);
        res.values.emplace(p.first,n);
      }

      res.objectiveValue = cplex.getObjValue()+objectiveOffset;
    }

    stat = mapStatus(cplex.getCplexStatus());
  }
  catch(IloCplex::Exception& e)
  {
    if(e.getStatus()==1117)
    {
      stat = ILP::status::INFEASIBLE;
    }
    else
    {
      throw ILP::Exception(e.getMessage());
    }
  }

  return stat;
}

void ILP::SolverCPLEX::reset()
{
  try
  {
    model = IloModel(env);
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }
}

void ILP::SolverCPLEX::setConsoleOutput(bool verbose)
{
  this->verbose=verbose;
}

void ILP::SolverCPLEX::setTimeout(long timeout)
{
  this->timeout=timeout;
}

void ILP::SolverCPLEX::presolve(bool presolve)
{
  try
  {
    this->presolving=presolve;
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }
}

void ILP::SolverCPLEX::setThreads(unsigned int t)
{
  this->threads=t;
}

void ILP::SolverCPLEX::setRelativeMIPGap(double d)
{
  this->relMIPGap=d;
  //this->absMIPGap=-1;
}

void ILP::SolverCPLEX::setAbsoluteMIPGap(double d)
{
  this->absMIPGap=d;
  //this->relMIPGap=-1;
}
