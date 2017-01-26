
#include <ILP/SolverBackend/SolverGurobi_intern.h>
#include <ILP/SolverBackend/SolverGurobi.h>

#include <limits>
#include <cmath>

#include <ILP/Exception.h>

ILP::SolverBackend* newSolverGurobi()
{
  return new ILP::SolverGurobi();
}

ILP::SolverBackend* ILP::newSolverGurobi()
{
  return ::newSolverGurobi();
}

ILP::SolverGurobi::SolverGurobi()
  :environment(GRBEnv()), model(GRBModel(environment))
{
  name="Gurobi";
}

char ILP::SolverGurobi::variableType(ILP::VariableBase::type t)
{
  switch (t)
  {
    case ILP::VariableBase::type::INTEGER: return GRB_INTEGER;
    case ILP::VariableBase::type::REAL:    return GRB_CONTINUOUS;
    case ILP::VariableBase::type::BINARY:  return GRB_BINARY;
    default: return GRB_INTEGER;
  }
}

bool ILP::SolverGurobi::addVariable(const ILP::Variable& v)
{
  GRBVar grbv;
  try
  {
    grbv = model.addVar(mapValue(v->lowerRange),mapValue(v->upperRange),0,variableType(v->usedType),v->name);
  }
  catch(GRBException e)
  {
    throw ILP::Exception("Error while adding Variable \""+ v->name + "\": " + e.getMessage());
  }
  variables.emplace(v,grbv);

  return true;
}

bool ILP::SolverGurobi::addVariables(ILP::VariableSet vs)
{
  for(const ILP::Variable v:vs)
  {
    addVariable(v); // can throw an ILP::Exception
  }

  try
  {
    model.update();
  }
  catch(GRBException e)
  {
    throw ILP::Exception("Error after adding Variables to the backend: "+e.getMessage());
  }
  return true;
}

static char mapRelation(ILP::relation r)
{
  char rel = GRB_LESS_EQUAL;
  switch(r)
  {
    case ILP::relation::LESS_EQ_THAN:
      return GRB_LESS_EQUAL;
    case ILP::relation::EQUAL:
      return GRB_EQUAL;
    case ILP::relation::MORE_EQ_THAN:
      return GRB_GREATER_EQUAL;
    default:
      throw ILP::Exception("Relation (" + ILP::Constraint::showRelation(r) + ") not supported at the moment.");
  }
  return GRB_LESS_EQUAL;
}

bool ILP::SolverGurobi::addConstraint(const ILP::Constraint& cons)
{
  try
  {
    switch(cons.ctype)
    {
      case ILP::Constraint::type::C2L:
        model.addConstr(cons.lbound,mapRelation(cons.lrel),mapTerm(cons.term),cons.name);
        break;
      case ILP::Constraint::type::C2R:
        model.addConstr(mapTerm(cons.term),mapRelation(cons.rrel),cons.ubound,cons.name);
        break;
      case ILP::Constraint::type::CEQ:
        model.addConstr(cons.lbound,mapRelation(cons.lrel),mapTerm(cons.term),cons.name);
        break;
      case ILP::Constraint::type::C3:
        if(cons.lrel==ILP::relation::LESS_EQ_THAN)
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
  catch(GRBException e)
  {
    throw ILP::Exception("Error while adding a Constraint to the backend: "+e.getMessage());
  }
  return true;
}

bool ILP::SolverGurobi::setObjective(ILP::Objective o)
{
  int type = o.usedType==ILP::Objective::type::MAXIMIZE ? GRB_MAXIMIZE : GRB_MINIMIZE;
  try
  {
    GRBLinExpr t = mapTerm(o.usedTerm);
    model.setObjective(t-o.usedTerm.constant,type);
    objectiveOffset = o.usedTerm.constant;
  }
  catch(GRBException e)
  {
    throw ILP::Exception("Error while setting the Objective in the backend: "+e.getMessage());
  }

  return true;
}

ILP::status ILP::SolverGurobi::solve()
{
  try
  {
    //model.write("gurobi.lp");

    model.optimize();
  
    int grbStatus = model.get(GRB_IntAttr_Status);

    res.objectiveValue = model.get(GRB_DoubleAttr_ObjVal)+objectiveOffset;

    if(grbStatus != GRB_INFEASIBLE)
    {
      for(auto &p:variables)
      {
        res.values.emplace(p.first,p.second.get(GRB_DoubleAttr_X));
      }
    }
    if(grbStatus == GRB_OPTIMAL) return ILP::status::OPTIMAL;
    if(grbStatus == GRB_UNBOUNDED) return ILP::status::UNBOUND;
    if(grbStatus == GRB_SUBOPTIMAL) return ILP::status::FEASIBLE;
    if(grbStatus == GRB_TIME_LIMIT) return ILP::status::TIMEOUT;
    if(grbStatus == GRB_INFEASIBLE) return ILP::status::INFEASIBLE;
  }
  catch(GRBException e)
  {
    throw ILP::Exception("Error while solving: "+e.getMessage());
  }

  return ILP::status::ERROR;
}

GRBLinExpr ILP::SolverGurobi::mapTerm(ILP::Term t)
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
      throw ILP::Exception("Gurobi trys to access a non-existing variable, this should never happen ("+p.first->name+")");
    }
    coeffs[i] = p.second;
    ++i;
  }
  expr.addTerms(coeffs.data(),vars.data(),t.sum.size());
  return expr;
}

double ILP::SolverGurobi::mapValue(double d)
{
  return d;
}

void ILP::SolverGurobi::reset()
{
  // clear the variables-cache
  variables.clear();

  // reset Gurobi itself
  try
  {
    model.reset();
  }catch(GRBException &e)
  {
    throw ILP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ILP::SolverGurobi::setConsoleOutput(bool verbose)
{
  try
  {
    model.getEnv().set(GRB_IntParam_OutputFlag,verbose);
  }catch(GRBException &e)
  {
    throw ILP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ILP::SolverGurobi::setTimeout(long timeout)
{
  try
  {
    model.getEnv().set(GRB_DoubleParam_TimeLimit,timeout);
  }catch(GRBException &e)
  {
    throw ILP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}

void ILP::SolverGurobi::presolve(bool presolve)
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
    model.presolve(); // TODO: <+ testing +> maybe the resolved Model is discarded
  }catch(GRBException &e)
  {
    throw ILP::Exception(std::to_string(e.getErrorCode())+" "+e.getMessage());
  }
}
