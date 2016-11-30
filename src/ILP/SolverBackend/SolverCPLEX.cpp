
#include <ILP/SolverBackend/SolverCPLEX.h>
#include <ILP/SolverBackend/SolverCPLEX_intern.h>

#include <ILP/Result.h>

#include <tuple>
#include <iostream>

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
}

static IloNumVar::Type mapVariableType(ILP::Variable& v)
{
  switch(v->usedType)
  {
    case ILP::VariableType::BINARY: return IloNumVar::Bool;
    case ILP::VariableType::INTEGER: return IloNumVar::Int;
    case ILP::VariableType::REAL: return IloNumVar::Float;
    default: throw ILP::Exception("Variable type not implemented yet.");
  }
  return ILOINT;
}

bool ILP::SolverCPLEX::addVariable(ILP::Variable v)
{
  bool r=false;
  try{
    auto p = variables.emplace(v,IloNumVar(env,v->lowerRange, v->upperRange,mapVariableType(v),v->name.c_str()));
    r=p.second;
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }
  return r;
}

static IloExpr mapTerm(IloEnv& env, const std::map<ILP::Variable,IloNumVar>& variables, const ILP::Term& t)
{
  IloExpr term= IloExpr(env)+t.constant;
  for(auto&p:t.sum)
  {
    term += variables.at(p.first)*p.second;
  }
  return term;
}

static std::tuple<double,IloExpr,double> addConstraint2(IloEnv& env, const std::map<ILP::Variable,IloNumVar>& variables, const ILP::Constraint2 &c)
{
  double lhs,rhs;
  const ILP::Term *sterm;;

  if(c.usedRelation==ILP::relation::LESS_EQ_THAN && c.lhs.sum.size()==0)
  { // a <= x
    lhs=c.lhs.constant;
    rhs=IloInfinity;
    sterm=&c.rhs;
  }
  else if(c.usedRelation==ILP::relation::LESS_EQ_THAN && c.rhs.sum.size()==0)
  { // x <= a
    lhs=-IloInfinity;
    rhs=c.rhs.constant;
    sterm=&c.lhs;
  }
  else if(c.usedRelation==ILP::relation::MORE_EQ_THAN && c.lhs.sum.size()==0)
  { // a >= x
    lhs=-IloInfinity;
    rhs=c.lhs.constant;
    sterm=&c.lhs;
  }
  else if(c.usedRelation==ILP::relation::MORE_EQ_THAN && c.rhs.sum.size()==0)
  { // x >= a
    lhs=c.rhs.constant;
    rhs=IloInfinity;
    sterm=&c.lhs;
  }
  else if(c.usedRelation==ILP::relation::MORE_EQ_THAN && c.lhs.sum.size()==0)
  { // a == x
    rhs=c.lhs.constant;
    lhs=c.lhs.constant;
    sterm=&c.rhs;
  }
  else if(c.usedRelation==ILP::relation::MORE_EQ_THAN && c.rhs.sum.size()==0)
  { // x == a
    rhs=c.rhs.constant;
    lhs=c.rhs.constant;
    sterm=&c.lhs;
  }
  else
  {
    throw ILP::Exception("This relation is not implemented yet.");
  }

  return std::make_tuple(lhs,mapTerm(env,variables,*sterm),rhs);
}

static std::tuple<double,IloExpr,double> addConstraint3(IloEnv& env, const std::map<ILP::Variable,IloNumVar>& variables, const ILP::Constraint3 &c)
{
  double lhs,rhs;
  if(c.lrel==ILP::relation::LESS_EQ_THAN && c.rrel==ILP::relation::LESS_EQ_THAN)
  { // a <= x <= b
    lhs=c.lbound.constant;
    rhs=c.ubound.constant;
  }
  else if(c.lrel==ILP::relation::MORE_EQ_THAN && c.rrel==ILP::relation::MORE_EQ_THAN)
  { // a >= x >= b
    lhs=c.ubound.constant;
    rhs=c.lbound.constant;
  }

  return std::make_tuple(lhs,mapTerm(env,variables,c.term),rhs);
}

bool ILP::SolverCPLEX::addConstraints(std::list<ILP::Constraint> cons)
{
  char* name=nullptr; // TODO: add names
  try
  {
    IloRangeArray cc(env);
    for(const ILP::Constraint &c:cons)
    {
      double lhs,rhs;
      IloExpr term;
      if(c.usedType == ILP::Constraint::type::Constraint_2)
      {
        std::tie(lhs,term,rhs) = addConstraint2(env,variables,c.c2);
      }
      else
      {
        std::tie(lhs,term,rhs) = addConstraint3(env,variables,c.c3);
      }
      cc.add(IloRange(env,lhs,term,rhs,name));
    }
    model.add(cc);
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
  }

  return true;
}

static ILP::status mapStatus(IloAlgorithm::Status s)
{
  switch(s)
  {
    case IloAlgorithm::Status::Unknown:    return ILP::status::ERROR;
    case IloAlgorithm::Status::Feasible:   return ILP::status::FEASIBLE;
    case IloAlgorithm::Status::Optimal:    return ILP::status::OPTIMAL;
    case IloAlgorithm::Status::Infeasible: return ILP::status::INFEASIBLE;
    case IloAlgorithm::Status::Unbounded:  return ILP::status::UNBOUND;
    case IloAlgorithm::Status::Error:      return ILP::status::ERROR;
    default: return ILP::status::ERROR;
  }
  return ILP::status::ERROR;
}

bool ILP::SolverCPLEX::setObjective(ILP::Objective o)
{
  try
  {
    IloExpr obj(env);
    for(auto&p:o.usedTerm.sum)
    {
      obj += variables.at(p.first) * p.second;
    }
    obj+=o.usedTerm.constant;

    if(o.usedType==ILP::Objective::type::MINIMIZE)
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
    if(!verbose) cplex.setOut(env.getNullStream());

    // set time limit
    if(timeout>0) cplex.setParam(IloCplex::TiLim,timeout);

    // simplify model if possible
    cplex.presolve(IloCplex::Algorithm::NoAlg);

    //cplex.exportModel("aaa.lp");

    if(cplex.solve())
    {
      // extract result
      for(auto&p:variables)
      {
        IloNum n = cplex.getValue(p.second);
        res.values.emplace(p.first,n);
      }
    }

    stat = mapStatus(cplex.getStatus());
  }
  catch(IloException& e)
  {
    throw ILP::Exception(e.getMessage());
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
  verbose=verbose;
}

void ILP::SolverCPLEX::setTimeout(long timeout)
{
  timeout=timeout;
}
