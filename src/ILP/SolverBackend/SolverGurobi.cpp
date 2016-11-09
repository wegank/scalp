
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

bool ILP::SolverGurobi::addVariable(ILP::Variable v)
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
      {
        rel = GRB_LESS_EQUAL;
        break;
      }
    case ILP::relation::EQUAL:
      {
        rel = GRB_EQUAL;
        break;
      }
    case ILP::relation::MORE_EQ_THAN:
      {
        rel = GRB_GREATER_EQUAL;
        break;
      }
    default: throw ILP::Exception("Relation (" + ILP::Constraint::showRelation(r) + ") not supported at the moment.");
  }
  return rel;
}

bool ILP::SolverGurobi::addConstraint2(ILP::Term lhs, ILP::relation r, ILP::Term rhs)
{
  try
  {
    model.addConstr(mapTerm(lhs),mapRelation(r),mapTerm(rhs));
  }
  catch(GRBException e)
  {
    throw ILP::Exception("Error while adding a Constraint to the backend: "+e.getMessage());
  }
  return true;
}

bool ILP::SolverGurobi::addConstraint3(ILP::Constraint3 &c3)
{
  try
  {
    // Gurobi Range
    model.addRange(mapTerm(c3.term),c3.lbound.constant,c3.ubound.constant);
  }
  catch(GRBException e)
  {
    throw ILP::Exception("Error while adding a Constraint to the backend: "+e.getMessage());
  }
  return true;
}

bool ILP::SolverGurobi::addConstraint(ILP::Constraint cons)
{
  if(cons.usedType==ILP::Constraint::type::Constraint_2)
    return addConstraint2(cons.c2.lhs,cons.c2.usedRelation,cons.c2.rhs);
  else if(cons.usedType==ILP::Constraint::type::Constraint_3)
    return addConstraint3(cons.c3);
  else
  {
    throw ILP::Exception("Constraint-Type not supported");
    return false;
  }
}

bool ILP::SolverGurobi::setObjective(ILP::Objective o)
{
  int type = o.usedType==ILP::Objective::type::MAXIMIZE ? GRB_MAXIMIZE : GRB_MINIMIZE;
  try
  {
    GRBLinExpr t = mapTerm(o.usedTerm);
    model.setObjective(t,type);
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
    model.write("aa.lp");

    model.optimize();
  
    int grbStatus = model.get(GRB_IntAttr_Status);

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
    vars[i] = variables.at(p.first);
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

