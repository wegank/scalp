
#include <ILP/SolverBackend/SolverSCIP.h>
#include <ILP/SolverBackend/SolverSCIP_intern.h>

#include <ILP/Exception.h>

#include <vector>
#include <utility>
#include <iostream>

namespace ILP
{
  extern double INF();
}

#define SCALP_SCIP_EXC(F) {\
  SCIP_RETCODE scalp_ret_code = F;\
  if(scalp_ret_code!=SCIP_OKAY)\
  {\
    throw ILP::Exception("SCIP-Error: "+ scalp_ret_code);\
  }\
}

ILP::SolverBackend* newSolverSCIP()
{
  return new ILP::SolverSCIP();
}

ILP::SolverBackend* ILP::newSolverSCIP()
{
  return ::newSolverSCIP();
}

ILP::SolverSCIP::SolverSCIP()
{
  name="SCIP";
  // creation is done in reset().
}

ILP::SolverSCIP::~SolverSCIP()
{
  if(scip!=nullptr)
  { // at least one run
    // FIXME: throwing in Destructor leads to warnings.
    //SCALP_SCIP_EXC(SCIPfree(&scip));
    SCIPfree(&scip);
  }
}

static SCIP_VARTYPE mapVariableType(ILP::VariableType t)
{
  switch(t)
  {
    case ILP::VariableType::BINARY:  return SCIP_VARTYPE_BINARY;
    case ILP::VariableType::INTEGER: return SCIP_VARTYPE_INTEGER;
    case ILP::VariableType::REAL:    return SCIP_VARTYPE_CONTINUOUS;
    default: throw ILP::Exception("Unknown variable type.");
  }
}

bool ILP::SolverSCIP::addVariable(const ILP::Variable& v)
{
  SCIP_VAR* var;
  SCALP_SCIP_EXC(SCIPcreateVarBasic(scip,&var,v->name.c_str(),
        v->lowerRange, v->upperRange,
        0.0, mapVariableType(v->usedType)));
  SCALP_SCIP_EXC(SCIPaddVar(scip, var));

  return variables.emplace(v,var).second; // inserted correctly?
}

// add the Constraint to scip, returns the added constraint.
static SCIP_CONS* scipAddCons(SCIP* scip, std::map<ILP::Variable,SCIP_VAR*> &variables, const ILP::Term& term, double lhs, double rhs, std::string name="")
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

bool ILP::SolverSCIP::addConstraint(const ILP::Constraint& c)
{

  if(c.lrel==ILP::relation::MORE_EQ_THAN and c.rrel==ILP::relation::MORE_EQ_THAN)
  { // flip boundaries
    constraints.push_back(scipAddCons(scip,variables,c.term,c.ubound,c.lbound,c.name.c_str()));
  }
  else
  {
    constraints.push_back(scipAddCons(scip,variables,c.term,c.lbound,c.ubound,c.name.c_str()));
  }
  return true;
}

bool ILP::SolverSCIP::setObjective(ILP::Objective o)
{
  if(o.usedType==ILP::Objective::type::MAXIMIZE)
  {
    SCALP_SCIP_EXC(SCIPsetObjsense(scip, SCIP_OBJSENSE_MAXIMIZE));
  }
  else
  {
    SCALP_SCIP_EXC(SCIPsetObjsense(scip, SCIP_OBJSENSE_MINIMIZE));
  }

  for(auto& p:o.usedTerm.sum)
  {
    SCALP_SCIP_EXC(SCIPchgVarObj(scip,variables.at(p.first),p.second));
  }

  objectiveOffset=o.usedTerm.constant;

  return true;
}

ILP::status ILP::SolverSCIP::solve()
{
  SCALP_SCIP_EXC(SCIPsolve(scip));
  SCIP_SOL* sol = SCIPgetBestSol(scip);

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
    case SCIP_STATUS_TIMELIMIT:     return ILP::status::TIMEOUT;
    case SCIP_STATUS_OPTIMAL:       return ILP::status::OPTIMAL;
    case SCIP_STATUS_INFEASIBLE:    return ILP::status::INFEASIBLE;
    default:
      {
        std::cerr << "Scalp: This SCIP-Status is not supported, please report with an simplified example" << std::endl;
        return ILP::status::ERROR;
      }
  }
  return ILP::status::ERROR;
}

void ILP::SolverSCIP::reset()
{
  if(scip!=nullptr) // clean up
  {
    for(auto&p:variables)
    {
      SCALP_SCIP_EXC(SCIPreleaseVar(scip,&p.second));
    }

    auto cons = constraints.data();
    SCIPreleaseCons(scip,cons);
  }
  scip=nullptr;

  // create new Instance
  SCALP_SCIP_EXC(SCIPcreate(&(this->scip)));
  SCALP_SCIP_EXC(SCIPincludeDefaultPlugins(scip));

  SCALP_SCIP_EXC(SCIPcreateProbBasic(scip,"scip"));
  SCALP_SCIP_EXC(SCIPsetIntParam(scip,"timing/clocktype",2));
}

void ILP::SolverSCIP::setConsoleOutput(bool verbose)
{
  SCIPsetMessagehdlrQuiet(scip,!verbose);
}

void ILP::SolverSCIP::setTimeout(long timeout)
{
  SCALP_SCIP_EXC(SCIPsetRealParam(scip, "limits/time", timeout));
}

void ILP::SolverSCIP::presolve(bool presolve)
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
