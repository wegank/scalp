
#include <ILP/SolverBackend/SolverSCIP.h>
#include <ILP/SolverBackend/SolverSCIP_intern.h>

#include <ILP/Exception.h>

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
  // creation is done in reset().
}

static SCIP_VARTYPE mapVariableType(ILP::VariableType t)
{
  if(t==ILP::VariableType::BINARY)
    return SCIP_VARTYPE_BINARY;
  else if(t==ILP::VariableType::INTEGER)
    return SCIP_VARTYPE_INTEGER;
  else if(t==ILP::VariableType::REAL)
    return SCIP_VARTYPE_CONTINUOUS;
  else
  {
    throw ILP::Exception("Unknown variable type.");
  }
}

bool ILP::SolverSCIP::addVariables(ILP::VariableSet vs)
{
  for(auto v:vs)
  {
    SCIP_VAR* var;
    SCALP_SCIP_EXC(SCIPcreateVarBasic(scip,&var,v->name.c_str(),
          v->lowerRange, v->upperRange,
          1.0, mapVariableType(v->usedType)));
    SCALP_SCIP_EXC(SCIPaddVar(scip, var));

    variables.emplace(v,var);
  }
  return true;
}

bool ILP::SolverSCIP::addConstraint(ILP::Constraint cons)
{
  //SCIP_CONS* cons;
  //SCALP_SCIP_EXC(SCIPcreateConsBasicLinear(scip,cons,0,nullptr,nullptr));
  //for(auto& p:cons.c2.usedTerm)
  //{
  //  SCALP_SCIP_EXC(SCIPaddCoefLinear(scip,cons,variables.at(p.first),p.second));
  //}
  return false;
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

  // TODO constant part missing.

  return true;
}

ILP::status ILP::SolverSCIP::solve()
{
  return ILP::status::ERROR;
}

void ILP::SolverSCIP::reset()
{
  SCALP_SCIP_EXC(SCIPcreate(&(this->scip)));
  SCALP_SCIP_EXC(SCIPincludeDefaultPlugins(scip));
  SCALP_SCIP_EXC(SCIPcreateProbBasic(scip,"scip"));
}

void ILP::SolverSCIP::setConsoleOutput(bool verbose)
{
  SCALP_SCIP_EXC(SCIPsetMessagehdlr(scip,nullptr));
}
