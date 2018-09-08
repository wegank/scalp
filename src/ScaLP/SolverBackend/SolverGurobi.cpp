
#include <ScaLP/SolverBackend/SolverGurobi_intern.h>
#include <ScaLP/SolverBackend/SolverGurobi.h>

#include <limits>
#include <cmath>

#include <ScaLP/Exception.h>

ScaLP::SolverBackend* newSolverGurobi()
{
  return new ScaLP::SolverGurobi();
}

ScaLP::SolverBackend* ScaLP::newSolverGurobi()
{
  return ::newSolverGurobi();
}

ScaLP::SolverGurobi::SolverGurobi()
try :environment(GRBEnv()), model(GRBModel(environment))
{

  name="Gurobi";
  this->features.lp=true;
  this->features.ilp=true;
  this->features.qp=false;
  this->features.milp=true;
  #if GRB_VERSION_MAJOR < 7
  this->features.indicators=false;
  #else
  this->features.indicators=true;
  #endif
  this->features.logical=false;
  this->features.warmstart=true;
}
catch(GRBException e)
{
  throw ScaLP::Exception("Error while creating Gurobi environment" + e.getMessage());
}

char ScaLP::SolverGurobi::variableType(ScaLP::VariableType t)
{
  switch (t)
  {
    case ScaLP::VariableType::INTEGER: return GRB_INTEGER;
    case ScaLP::VariableType::REAL:    return GRB_CONTINUOUS;
    case ScaLP::VariableType::BINARY:  return GRB_BINARY;
    default: return GRB_INTEGER;
  }
}

bool ScaLP::SolverGurobi::addVariable(const ScaLP::Variable& v)
{
  GRBVar grbv;
  try
  {
    grbv = model.addVar(mapValue(v->getLowerBound()),mapValue(v->getUpperBound()),0,variableType(v->getType()),v->getName());
  }
  catch(GRBException e)
  {
    throw ScaLP::Exception("Error while adding Variable \""+ v->getName() + "\": " + e.getMessage());
  }
  variables.emplace(v,grbv);

  return true;
}

bool ScaLP::SolverGurobi::addVariables(const ScaLP::VariableSet& vs)
{
  for(const ScaLP::Variable v:vs)
  {
    addVariable(v); // can throw an ScaLP::Exception
  }

  try
  {
    model.update();
  }
  catch(GRBException e)
  {
    throw ScaLP::Exception("Error after adding Variables to the backend: "+e.getMessage());
  }
  return true;
}

static char mapRelation(ScaLP::relation r)
{
  char rel = GRB_LESS_EQUAL;
  switch(r)
  {
    case ScaLP::relation::LESS_EQ_THAN:
      return GRB_LESS_EQUAL;
    case ScaLP::relation::EQUAL:
      return GRB_EQUAL;
    case ScaLP::relation::MORE_EQ_THAN:
      return GRB_GREATER_EQUAL;
    default:
      throw ScaLP::Exception("Relation (" + ScaLP::Constraint::showRelation(r) + ") not supported at the moment.");
  }
  return GRB_LESS_EQUAL;
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

bool ScaLP::SolverGurobi::addConstraint(const ScaLP::Constraint& cons)
{
  try
  {
    // TODO: check if avoiding temporary constraints increase performance
    if(cons.indicator==nullptr)
    {
      switch(cons.ctype)
      {
        case ScaLP::Constraint::type::C2L:
          model.addConstr(cons.lbound,mapRelation(cons.lrel),mapTerm(cons.term),cons.name);
          break;
        case ScaLP::Constraint::type::C2R:
          model.addConstr(mapTerm(cons.term),mapRelation(cons.rrel),cons.ubound,cons.name);
          break;
        case ScaLP::Constraint::type::CEQ:
          model.addConstr(cons.lbound,mapRelation(cons.lrel),mapTerm(cons.term),cons.name);
          break;
        case ScaLP::Constraint::type::C3:
          if(cons.lrel==ScaLP::relation::LESS_EQ_THAN)
          { // d <= x <= d
            model.addRange(mapTerm(cons.term),cons.lbound,cons.ubound,cons.name);
          }
          else
          { // d >= x >= d
            model.addRange(mapTerm(cons.term),cons.ubound,cons.lbound,cons.name);
          }
          break;
      }
    }
    else
    {
    #if GRB_VERSION_MAJOR < 7
      throw ScaLP::Exception("ScaLP: Gurobi version "
        + std::to_string(GRB_VERSION_MAJOR)+ "."
        + std::to_string(GRB_VERSION_MINOR)+ "."
        + std::to_string(GRB_VERSION_TECHNICAL)
        + " does not support indicator-constraints");
    #else
      GRBVar var = variables[cons.indicator->term.sum.begin()->first];
      int val = cons.indicator->lbound;
      GRBLinExpr t = mapTerm(cons.term);
      switch(cons.ctype)
      {
        case ScaLP::Constraint::type::C2L:
          model.addGenConstrIndicator(var,val,t,mapRelation(flipRelation(cons.lrel)),cons.lbound,cons.name);
          break;
        case ScaLP::Constraint::type::C2R:
          model.addGenConstrIndicator(var,val,t,mapRelation(cons.rrel),cons.ubound,cons.name);
          break;
        case ScaLP::Constraint::type::CEQ:
          model.addGenConstrIndicator(var,val,t,mapRelation(cons.lrel),cons.lbound,cons.name);
          break;
        case ScaLP::Constraint::type::C3:
          model.addGenConstrIndicator(var,val,t,mapRelation(flipRelation(cons.lrel)),cons.lbound,cons.name+"_l");
          model.addGenConstrIndicator(var,val,t,mapRelation(cons.rrel),cons.ubound,cons.name+"_r");
          break;
      }
    #endif
    }
  }
  catch(GRBException e)
  {
    throw ScaLP::Exception("Error while adding a Constraint to the backend: "+e.getMessage());
  }
  return true;
}

bool ScaLP::SolverGurobi::setObjective(ScaLP::Objective o)
{
  int type = o.getType()==ScaLP::Objective::type::MAXIMIZE ? GRB_MAXIMIZE : GRB_MINIMIZE;
  try
  {
    GRBLinExpr t = mapTerm(o.getTerm());
    model.setObjective(t-o.getTerm().constant,type);
    objectiveOffset = o.getTerm().constant;
  }
  catch(GRBException e)
  {
    throw ScaLP::Exception("Error while setting the Objective in the backend: "+e.getMessage());
  }

  return true;
}

std::pair<ScaLP::status,ScaLP::Result> ScaLP::SolverGurobi::solve()
{
  ScaLP::Result res;
  try
  {
    model.optimize();

  
    int grbStatus = model.get(GRB_IntAttr_Status);

    if(grbStatus == GRB_OPTIMAL or grbStatus == GRB_SUBOPTIMAL or grbStatus == GRB_TIME_LIMIT)
    {
      if(model.get(GRB_IntAttr_SolCount)>0)
      {
        res.objectiveValue = model.get(GRB_DoubleAttr_ObjVal)+objectiveOffset;
        for(auto &p:variables)
        {
          res.values.emplace(p.first,p.second.get(GRB_DoubleAttr_X));
        }
      }
    }

    if(grbStatus == GRB_OPTIMAL) return {ScaLP::status::OPTIMAL,res};
    if(grbStatus == GRB_UNBOUNDED) return {ScaLP::status::UNBOUND,res};
    if(grbStatus == GRB_SUBOPTIMAL) return {ScaLP::status::FEASIBLE,res};
    if(grbStatus == GRB_TIME_LIMIT and model.get(GRB_IntAttr_SolCount)>0)
      return {ScaLP::status::TIMEOUT_FEASIBLE,res};
    if(grbStatus == GRB_TIME_LIMIT) return {ScaLP::status::TIMEOUT_INFEASIBLE,res};
    if(grbStatus == GRB_INFEASIBLE) return {ScaLP::status::INFEASIBLE,res};
    if(grbStatus == GRB_INF_OR_UNBD) return {ScaLP::status::INFEASIBLE_OR_UNBOUND,res};
  }
  catch(GRBException e)
  {
    throw ScaLP::Exception("Error while solving: "+e.getMessage());
  }

  return {ScaLP::status::ERROR,res};
}

GRBLinExpr ScaLP::SolverGurobi::mapTerm(ScaLP::Term t)
{
  GRBLinExpr expr = t.constant;
  std::vector<double> coeffs(t.sum.size());
  std::vector<GRBVar> vars(t.sum.size());

  int i=0;
  for(auto &p:t.sum)
  {
    try
    {
      vars[i] = variables.at(p.first);
    }
    catch(std::out_of_range& e)
    {
      throw ScaLP::Exception("Gurobi trys to access a non-existing variable, this should never happen ("+p.first->getName()+")");
    }
    coeffs[i] = p.second;
    ++i;
  }
  try
  {
    expr.addTerms(coeffs.data(),vars.data(),t.sum.size());
  }
  catch(GRBException e)
  {
    throw ScaLP::Exception("Error while converting a term for Gurobi: " + e.getMessage());
  }
  return expr;
}

double ScaLP::SolverGurobi::mapValue(double d)
{
  return d;
}

void ScaLP::SolverGurobi::reset()
{
  // clear the variables-cache
  variables.clear();
  objectiveOffset=0;

  // reset Gurobi itself
  try
  {
    model.~GRBModel();
    new (&model) GRBModel(environment);
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setConsoleOutput(bool verbose)
{
  try
  {
    model.getEnv().set(GRB_IntParam_OutputFlag,verbose);
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setTimeout(long timeout)
{
  try
  {
    model.getEnv().set(GRB_DoubleParam_TimeLimit,timeout);
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setIntFeasTol(double intFeasTol)
{
  try
  {
    model.getEnv().set(GRB_DoubleParam_IntFeasTol,intFeasTol);
//    model.getEnv().set(GRB_DoubleParam_FeasibilityTol,intFeasTol);

    std::cout << "!!! GRB_DoubleParam_IntFeasTol=" << model.getEnv().get(GRB_DoubleParam_IntFeasTol) << std::endl;
//    std::cout << "!!! GRB_DoubleParam_FeasibilityTol=" << model.getEnv().get(GRB_DoubleParam_FeasibilityTol) << std::endl;
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::presolve(bool presolve)
{
  try
  {
    if(presolve)
    {
      model.getEnv().set(GRB_IntParam_Presolve,2); // set aggressive presolve
    }
    else
    {
      model.getEnv().set(GRB_IntParam_Presolve,0); // no presolve
    }
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setThreads(unsigned int t)
{
  try
  {
    model.getEnv().set(GRB_IntParam_Threads,t);
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setRelativeMIPGap(double d)
{
  try
  {
    model.getEnv().set(GRB_DoubleParam_MIPGap,d);
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setAbsoluteMIPGap(double d)
{
  try
  {
    model.getEnv().set(GRB_DoubleParam_MIPGapAbs,d);
  }catch(GRBException &e)
  {
    throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ScaLP::SolverGurobi::setStartValues(const ScaLP::Result& start)
{
  for(auto&p:start.values)
  {
    try
    {
      variables.at(p.first).set(GRB_DoubleAttr_Start,p.second);
    }catch(GRBException &e)
    {
      throw ScaLP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
    }
  }
}
