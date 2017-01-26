
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
  switch(v->usedType)
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
    auto p = variables.emplace(v,IloNumVar(env,v->lowerRange, v->upperRange,mapVariableType(v),v->name.c_str()));
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

IloRange ILP::SolverCPLEX::createRange(double d, ILP::relation rel,const ILP::Term& t)
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
IloRange ILP::SolverCPLEX::createRange(const ILP::Term& t, ILP::relation rel,double d)
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

IloRange ILP::SolverCPLEX::createConstraint3(const ILP::Constraint& c)
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

bool ILP::SolverCPLEX::addConstraints(std::list<ILP::Constraint> cons)
{
  try
  {
    IloRangeArray cc(env);
    for(const ILP::Constraint &c:cons)
    {
      IloRange constr;
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
      cc.add(constr);
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
    objectiveOffset=o.usedTerm.constant;

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
    if(verbose) cplex.setOut(env.getNullStream());

    // set time limit
    if(timeout>0) cplex.setParam(IloCplex::TiLim,timeout);

    // simplify model if possible
    if(presolving)
    {
      cplex.setParam(IloCplex::PreInd,presolving);
      cplex.presolve(IloCplex::Algorithm::NoAlg);
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

      res.objectiveValue = cplex.getBestObjValue()+objectiveOffset;
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
