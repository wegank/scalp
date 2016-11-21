
#include <ILP/SolverBackend/SolverSCIP.h>
#include <ILP/SolverBackend/SolverSCIP_intern.h>

#include <ILP/Exception.h>

#include <vector>

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
  // creation is done in reset().
}

ILP::SolverSCIP::~SolverSCIP()
{
  if(scip!=nullptr) // at least one run
    SCALP_SCIP_EXC(SCIPfree(&scip));
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

bool ILP::SolverSCIP::addVariables(ILP::VariableSet vs)
{
  for(auto& v:vs)
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

static bool addConstraint2(SCIP* scip, std::map<ILP::Variable,SCIP_VAR*> &variables, ILP::Constraint2 c)
{
  SCIP_CONS* cons;

  double lhs,rhs;
  std::vector<SCIP_VAR*> vars;
  std::vector<double> vals;
  ILP::Term *term;

  if(!c.lhs.isConstant() && c.rhs.isConstant())
  {
    lhs=-ILP::INF();
    rhs=c.rhs.constant;
    term=&c.lhs;
    //for(auto&p:c.l.sum)
    //{
    //  vars.push_back(variables.at(p.first));
    //  vals.push_back(p.second);
    //}
  }
  else if(c.rhs.isConstant() && !c.lhs.isConstant())
  {
    lhs=c.lhs.constant;
    rhs=ILP::INF();
    term=&c.rhs;
  }
  else
  {
    throw ILP::Exception("Cant add Constraint, this should never happen.");
  }

  // build the arrays.
  for(auto&p:term->sum)
  {
    vars.push_back(variables.at(p.first));
    vals.push_back(p.second);
  }

  // TODO: Constant part is missing.
  // TODO: Constraint names
  // TODO: use Relation
  
  SCIP_VAR** varss=vars.data();
  double*valss=vals.data();
  SCALP_SCIP_EXC(SCIPcreateConsBasicLinear(scip,&cons,"",vars.size(),varss,valss,lhs,rhs));
  //SCALP_SCIP_EXC(SCIPcreateConsBasicLinear(scip,cons,vars.size(),&vars.data(),&vals.data(),lhs,rhs));
  SCALP_SCIP_EXC(SCIPaddCons(scip,cons));

  return true;
}

bool ILP::SolverSCIP::addConstraint(ILP::Constraint c)
{
  if(c.usedType==ILP::Constraint::type::Constraint_2)
  {
    return addConstraint2(scip,variables,c.c2);
  }
  else
  {
    return false;
  }
}
// SCIPcreateConsBasicLinear (SCIP *scip, SCIP_CONS **cons, const char *name, int nvars, SCIP_VAR **vars, SCIP_Real *vals, SCIP_Real lhs, SCIP_Real rhs)

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
  SCALP_SCIP_EXC(SCIPsolve(scip));
  SCIP_SOL* sol = SCIPgetBestSol(scip);

  // TODO:
  if(sol==nullptr)
    return ILP::status::ERROR;

  else
    return ILP::status::OPTIMAL;
}

void ILP::SolverSCIP::reset()
{
  SCALP_SCIP_EXC(SCIPcreate(&(this->scip)));
  SCALP_SCIP_EXC(SCIPincludeDefaultPlugins(scip));

  for(auto&p:variables)
  {
    SCALP_SCIP_EXC(SCIPreleaseVar(scip,&p.second));
  }
  // TODO: remove constraints (needs an datastructure)
  //SCALP_SCIP_EXC(SCIPreleaseCons(scip, & cons);

  SCALP_SCIP_EXC(SCIPcreateProbBasic(scip,"scip"));
}

void ILP::SolverSCIP::setConsoleOutput(bool verbose)
{
  SCALP_SCIP_EXC(SCIPsetMessagehdlr(scip,nullptr));
}
