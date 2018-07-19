
#include <ScaLP/SolverBackend/SolverCPLEX.h>
#include <ScaLP/SolverBackend/SolverCPLEX_intern.h>

#include <ScaLP/Result.h>

#include <tuple>

ScaLP::SolverBackend* newSolverCPLEX()
{
  return new ScaLP::SolverCPLEX();
}

ScaLP::SolverBackend* ScaLP::newSolverCPLEX()
{
  return ::newSolverCPLEX();
}

ScaLP::SolverCPLEX::SolverCPLEX()
  :env(IloEnv()), model(IloModel(env))
{
  name="CPLEX";
  this->features.lp=true;
  this->features.ilp=true;
  this->features.qp=false;
  this->features.milp=true;
  this->features.indicators=true;
  this->features.logical=false;
  this->features.warmstart=true;
}

static IloNumVar::Type mapVariableType(const ScaLP::Variable& v)
{
  switch(v->getType())
  {
    case ScaLP::VariableType::BINARY: return IloNumVar::Bool;
    case ScaLP::VariableType::INTEGER: return IloNumVar::Int;
    case ScaLP::VariableType::REAL: return IloNumVar::Float;
    default: throw ScaLP::Exception("Variable type not implemented yet.");
  }
  return ILOINT;
}

bool ScaLP::SolverCPLEX::addVariable(const ScaLP::Variable& v)
{
  bool r=false;
  try{
    auto p = variables.emplace(v,IloNumVar(env,v->getLowerBound(), v->getUpperBound(),mapVariableType(v),v->getName().c_str()));
    r=p.second;
  }
  catch(IloException& e)
  {
    throw ScaLP::Exception(e.getMessage());
  }
  return r;
}

IloExpr ScaLP::SolverCPLEX::mapTerm(const ScaLP::Term& t)
{
  IloExpr term= IloExpr(env)+t.constant;
  for(auto&p:t.sum)
  {
    term += variables.at(p.first)*p.second;
  }
  return term;
}

IloConstraint ScaLP::SolverCPLEX::createRange(double d, ScaLP::relation rel,const ScaLP::Term& t)
{
  switch(rel)
  {
    case ScaLP::relation::MORE_EQ_THAN:
      return d>=mapTerm(t);
    case ScaLP::relation::LESS_EQ_THAN:
      return d<=mapTerm(t);
    case ScaLP::relation::EQUAL:
      return d==mapTerm(t);
  }
}
IloConstraint ScaLP::SolverCPLEX::createRange(const ScaLP::Term& t, ScaLP::relation rel,double d)
{
  switch(rel)
  {
    case ScaLP::relation::MORE_EQ_THAN:
      return mapTerm(t)>=d;
    case ScaLP::relation::LESS_EQ_THAN:
      return mapTerm(t)<=d;
    case ScaLP::relation::EQUAL:
      return mapTerm(t)==d;
  }
}

IloConstraint ScaLP::SolverCPLEX::createConstraint3(const ScaLP::Constraint& c)
{
  if(c.lrel==ScaLP::relation::LESS_EQ_THAN && c.rrel==ScaLP::relation::LESS_EQ_THAN)
  { // a <= x <= b
    return c.lbound <= mapTerm(c.term) <= c.ubound;
  }
  else if(c.lrel==ScaLP::relation::MORE_EQ_THAN && c.rrel==ScaLP::relation::MORE_EQ_THAN)
  { // a >= x >= b
    // FIXME: does not work (bug in CPLEX?)
    //return c.lbound >= mapTerm(c.term) >= c.ubound;
    return c.ubound <= mapTerm(c.term) <= c.lbound;
  }
}

IloConstraint ScaLP::SolverCPLEX::convertConstraint(const ScaLP::Constraint &c)
{
  IloConstraint constr;
  switch(c.ctype)
  {
    case ScaLP::Constraint::type::C3:
      constr = createConstraint3(c);
      break;
    case ScaLP::Constraint::type::C2L:
      constr = createRange(c.lbound,c.lrel,c.term);
      break;
    case ScaLP::Constraint::type::C2R:
      constr = createRange(c.term,c.rrel,c.ubound);
      break;
    case ScaLP::Constraint::type::CEQ:
      constr = createRange(c.lbound,c.lrel,c.term);
      break;
  }
  constr.setName(c.name.c_str());
  return constr;
}

bool ScaLP::SolverCPLEX::addConstraints(const std::vector<ScaLP::Constraint>& cons)
{
  try
  {
    IloConstraintArray cc(env);
    for(const ScaLP::Constraint &c:cons)
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
    throw ScaLP::Exception(e.getMessage());
  }

  return true;
}

static ScaLP::status mapStatus(IloCplex::CplexStatus s,bool solution)
{
  switch(s)
  {
    case IloCplex::CplexStatus::AbortTimeLim:
      {
        if(solution) return ScaLP::status::TIMEOUT_FEASIBLE;
        else         return ScaLP::status::TIMEOUT_INFEASIBLE;
      }
    case IloCplex::CplexStatus::Optimal:    return ScaLP::status::OPTIMAL;
    case IloCplex::CplexStatus::Unknown:    return ScaLP::status::UNKNOWN;
    case IloCplex::CplexStatus::Feasible:   return ScaLP::status::FEASIBLE;
    case IloCplex::CplexStatus::Infeasible: return ScaLP::status::INFEASIBLE;
    case IloCplex::CplexStatus::Unbounded:  return ScaLP::status::UNBOUND;
    case IloCplex::CplexStatus::InfOrUnbd:  return ScaLP::status::INFEASIBLE_OR_UNBOUND;
    default: return ScaLP::status::UNKNOWN;
  }
  return ScaLP::status::UNKNOWN;
}

bool ScaLP::SolverCPLEX::setObjective(ScaLP::Objective o)
{
  try
  {
    IloExpr obj(env);
    for(auto&p:o.getTerm().sum)
    {
      obj += variables.at(p.first) * p.second;
    }
    objectiveOffset=o.getTerm().constant;

    if(o.getType()==ScaLP::Objective::type::MINIMIZE)
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
    throw ScaLP::Exception(e.getMessage());
  }
  return true;
}

std::pair<ScaLP::status,ScaLP::Result> ScaLP::SolverCPLEX::solve()
{
  ScaLP::status stat = ScaLP::status::ERROR;
  ScaLP::Result res;
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

    // add warmstart
    if(startVar!=nullptr and startVal!=nullptr)
    {
      cplex.addMIPStart(*startVar, *startVal);
      startVal->end();
      startVar->end();
      delete startVal;
      delete startVar;
    }

    if(cplex.solve())
    {
      // extract result
      for(auto&p:variables)
      {
        IloNum n = cplex.getValue(p.second);
        res.values.emplace(p.first,n);
      }

      res.objectiveValue = cplex.getObjValue()+objectiveOffset;
      stat = mapStatus(cplex.getCplexStatus(),true);
    }
    else
    {
      stat = mapStatus(cplex.getCplexStatus(),false);
    }

  }
  catch(IloCplex::Exception& e)
  {
    if(e.getStatus()==1117)
    {
      stat = ScaLP::status::INFEASIBLE;
    }
    else
    {
      throw ScaLP::Exception(e.getMessage());
    }
  }

  return {stat,res};
}

void ScaLP::SolverCPLEX::reset()
{
  variables.clear();
  verbose=true;
  timeout=0;
  presolving=false;
  threads=0;
  relMIPGap=-1;
  absMIPGap=-1;
  objectiveOffset=0;

  try
  {
    env.end();
    new (&env)IloEnv();
    model = IloModel(env);
  }
  catch(IloException& e)
  {
    throw ScaLP::Exception(e.getMessage());
  }
}

void ScaLP::SolverCPLEX::setConsoleOutput(bool verbose)
{
  this->verbose=verbose;
}

void ScaLP::SolverCPLEX::setTimeout(long timeout)
{
  this->timeout=timeout;
}

void ScaLP::SolverCPLEX::presolve(bool presolve)
{
  try
  {
    this->presolving=presolve;
  }
  catch(IloException& e)
  {
    throw ScaLP::Exception(e.getMessage());
  }
}

void ScaLP::SolverCPLEX::setThreads(unsigned int t)
{
  this->threads=t;
}

void ScaLP::SolverCPLEX::setRelativeMIPGap(double d)
{
  this->relMIPGap=d;
  //this->absMIPGap=-1;
}

void ScaLP::SolverCPLEX::setAbsoluteMIPGap(double d)
{
  this->absMIPGap=d;
  //this->relMIPGap=-1;
}

void ScaLP::SolverCPLEX::setStartValues(const ScaLP::Result& start)
{
  // cleanup old values
  if(startVar!=nullptr)
  {
    startVar->end();
    delete startVar;
    startVar=nullptr;
  }
  if(startVal!=nullptr)
  {
    startVal->end();
    delete startVal;
    startVar=nullptr;
  }

  // add new values
  startVar = new IloNumVarArray(env);
  startVal = new IloNumArray(env);
  for(auto&p:start.values)
  {
    startVar->add(variables.at(p.first));
    startVal->add(p.second);
  }
}
