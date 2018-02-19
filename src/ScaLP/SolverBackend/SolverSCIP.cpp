
#include <ScaLP/SolverBackend/SolverSCIP.h>
#include <ScaLP/SolverBackend/SolverSCIP_intern.h>

#include <ScaLP/Exception.h>

#include <vector>
#include <utility>
#include <iostream>
#include <string>

namespace ScaLP
{
  extern double INF();
}

#define SCALP_SCIP_EXC(F) {\
  SCIP_RETCODE scalp_ret_code = F;\
  if(scalp_ret_code!=SCIP_OKAY)\
  {\
    throw ScaLP::Exception(std::string("SCIP-Error: ") + std::to_string(scalp_ret_code));\
  }\
}

ScaLP::SolverBackend* newSolverSCIP()
{
  return new ScaLP::SolverSCIP();
}

ScaLP::SolverBackend* ScaLP::newSolverSCIP()
{
  return ::newSolverSCIP();
}

ScaLP::SolverSCIP::SolverSCIP()
{
  name="SCIP";
  // creation is done in reset().
  this->features.lp=true;
  this->features.ilp=true;
  this->features.qp=false;
  this->features.milp=true;
  this->features.indicators=false;
  this->features.logical=false;
}

ScaLP::SolverSCIP::~SolverSCIP()
{
  if(scip!=nullptr)
  { // at least one run
    // FIXME: throwing in Destructor leads to warnings.
    //SCALP_SCIP_EXC(SCIPfree(&scip));
    for(auto&p:variables)
    {
      SCIPreleaseVar(scip,&p.second);
    }
    auto cons = constraints.data();
    for(unsigned int i=0;i<constraints.size();++i)
    {
      SCIPreleaseCons(scip,&(cons[i]));
    }
    SCIPfree(&scip);
  }
}

static SCIP_VARTYPE mapVariableType(ScaLP::VariableType t)
{
  switch(t)
  {
    case ScaLP::VariableType::BINARY:  return SCIP_VARTYPE_BINARY;
    case ScaLP::VariableType::INTEGER: return SCIP_VARTYPE_INTEGER;
    case ScaLP::VariableType::REAL:    return SCIP_VARTYPE_CONTINUOUS;
    default: throw ScaLP::Exception("Unknown variable type.");
  }
}

bool ScaLP::SolverSCIP::addVariable(const ScaLP::Variable& v)
{
  SCIP_VAR* var;
  SCALP_SCIP_EXC(SCIPcreateVarBasic(scip,&var,v->getName().c_str(),
        v->getLowerBound(), v->getUpperBound(),
        0.0, mapVariableType(v->getType())));
  SCALP_SCIP_EXC(SCIPaddVar(scip, var));

  return variables.emplace(v,var).second; // inserted correctly?
}

// add the Constraint to scip, returns the added constraint.
static SCIP_CONS* scipAddCons(SCIP* scip, std::map<ScaLP::Variable,SCIP_VAR*> &variables, const ScaLP::Term& term, double lhs, double rhs, std::string name="")
{
  SCIP_CONS* cons= nullptr;
  std::vector<SCIP_VAR*> vars;
  std::vector<double> vals;

  for(auto&p:term.sum)
  {
    vars.push_back(variables.at(p.first));
    vals.push_back(p.second);
  }

  if(term.constant!=0)
  {
    lhs-=term.constant;
    rhs-=term.constant;
  }

  SCALP_SCIP_EXC(SCIPcreateConsBasicLinear(scip,&cons,name.c_str(),vars.size(),vars.data(),vals.data(),lhs,rhs));
  SCALP_SCIP_EXC(SCIPaddCons(scip,cons));

  return cons;
}

bool ScaLP::SolverSCIP::addConstraint(const ScaLP::Constraint& c)
{
  if(c.indicator!=nullptr)
  {
    throw ScaLP::Exception("Indicator-Constraints are not supported at the moment for SCIP");
  }

  switch(c.ctype)
  {
    case ScaLP::Constraint::type::C2L:
    {
      if(c.lrel==ScaLP::relation::LESS_EQ_THAN)
      {
        constraints.push_back(scipAddCons(scip,variables,c.term,c.lbound,ScaLP::INF(),c.name.c_str()));
      }
      else
      {
        constraints.push_back(scipAddCons(scip,variables,c.term,c.lbound,c.ubound,c.name.c_str()));
      }
      break;
    }
    case ScaLP::Constraint::type::C2R:
    {
      if(c.rrel==ScaLP::relation::LESS_EQ_THAN)
      {
        constraints.push_back(scipAddCons(scip,variables,c.term,c.lbound, c.ubound,c.name.c_str()));
      }
      else
      {
        constraints.push_back(scipAddCons(scip,variables,c.term,c.ubound,ScaLP::INF(),c.name.c_str()));
      }
      break;
    }
    case ScaLP::Constraint::type::CEQ:
    {
        constraints.push_back(scipAddCons(scip,variables,c.term,c.lbound, c.lbound,c.name.c_str()));
      break;
    }
    case ScaLP::Constraint::type::C3:
    {
      if(c.lrel==ScaLP::relation::MORE_EQ_THAN and c.rrel==ScaLP::relation::MORE_EQ_THAN)
      { // flip boundaries
        constraints.push_back(scipAddCons(scip,variables,c.term,c.ubound,c.lbound,c.name.c_str()));
      }
      else
      {
        constraints.push_back(scipAddCons(scip,variables,c.term,c.lbound,c.ubound,c.name.c_str()));
      }
      break;
    }
  }
  return true;
}

bool ScaLP::SolverSCIP::setObjective(ScaLP::Objective o)
{
  if(o.getType()==ScaLP::Objective::type::MAXIMIZE)
  {
    SCALP_SCIP_EXC(SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE));
  }
  else
  {
    SCALP_SCIP_EXC(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));
  }

  for(auto& p:o.getTerm().sum)
  {
    SCALP_SCIP_EXC(SCIPchgVarObj(scip,variables.at(p.first),p.second));
  }

  objectiveOffset=o.getTerm().constant;

  return true;
}

std::pair<ScaLP::status,ScaLP::Result> ScaLP::SolverSCIP::solve()
{
  SCALP_SCIP_EXC(SCIPsolve(scip));
  SCIP_SOL* sol = SCIPgetBestSol(scip);
  ScaLP::Result res;

  // Objective-value
  res.objectiveValue = SCIPgetPrimalbound(scip) + objectiveOffset;

  if(sol!=nullptr)
  {
    for(auto &p:variables)
    {
      double variableValue = SCIPgetSolVal(scip,sol,p.second);
      res.values.emplace(p.first,variableValue);
    }

    // TODO: <+ SCIP apparently does not support Constraint-solutions +>
    // TODO: <+ Maybe create them out of the variable solutions?  +>
  }

  switch(SCIPgetStatus(scip))
  {
    case SCIP_STATUS_TIMELIMIT:
      {
        if(sol!=nullptr) return {ScaLP::status::TIMEOUT_FEASIBLE,res};
        else             return {ScaLP::status::TIMEOUT_INFEASIBLE,res};
      }
    case SCIP_STATUS_OPTIMAL:    return {ScaLP::status::OPTIMAL,res};
    case SCIP_STATUS_INFEASIBLE: return {ScaLP::status::INFEASIBLE,res};
    default:
      {
        std::cerr << "Scalp: This SCIP-Status is not supported, please report with an simplified example" << std::endl;
        return {ScaLP::status::ERROR,res};
      }
  }
  return {ScaLP::status::ERROR,res};
}

void ScaLP::SolverSCIP::reset()
{
  if(scip!=nullptr) // clean up
  {
    for(auto&p:variables)
    {
      SCALP_SCIP_EXC(SCIPreleaseVar(scip,&p.second));
    }

    auto cons = constraints.data();
    for(unsigned int i=0;i<constraints.size();++i)
    {
      SCALP_SCIP_EXC(SCIPreleaseCons(scip,&(cons[i])));
    }
    SCALP_SCIP_EXC(SCIPfree(&scip));
    scip=nullptr;
  }

  constraints.clear();
  variables.clear();
  objectiveOffset=0;

  // create new Instance
  SCALP_SCIP_EXC(SCIPcreate(&(this->scip)));
  SCALP_SCIP_EXC(SCIPincludeDefaultPlugins(scip));

  SCALP_SCIP_EXC(SCIPcreateProbBasic(scip,"scip"));
  SCALP_SCIP_EXC(SCIPsetIntParam(scip,"timing/clocktype",2));
}

void ScaLP::SolverSCIP::setConsoleOutput(bool verbose)
{
  SCIPsetMessagehdlrQuiet(scip,!verbose);
}

void ScaLP::SolverSCIP::setTimeout(long timeout)
{
  SCALP_SCIP_EXC(SCIPsetRealParam(scip, "limits/time", timeout));
}

void ScaLP::SolverSCIP::presolve(bool presolve)
{
  if(presolve)
  {
    SCALP_SCIP_EXC(SCIPsetPresolving(scip,SCIP_PARAMSETTING_DEFAULT,true));
  }
  else
  {
    SCALP_SCIP_EXC(SCIPsetPresolving(scip,SCIP_PARAMSETTING_OFF,true));
  }
}

void ScaLP::SolverSCIP::setThreads(unsigned int t)
{
  SCALP_SCIP_EXC(SCIPsetIntParam(scip,"lp/threads",t));
}
