
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

bool ILP::SolverSCIP::addVariable(ILP::Variable v)
{
  SCIP_VAR* var;
  SCALP_SCIP_EXC(SCIPcreateVarBasic(scip,&var,v->name.c_str(),
        v->lowerRange, v->upperRange,
        0.0, mapVariableType(v->usedType)));
  SCALP_SCIP_EXC(SCIPaddVar(scip, var));

  auto r=variables.emplace(v,var);
  return r.second; // inserted correctly
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

  // EXPERIMENTAL: equivalent transformation (remove the constant part from the term)
  if(term.constant!=0)
  {
    lhs-=term.constant;
    rhs-=term.constant;
    //throw ILP::Exception("Constant parts of Constraints are not implemented yet.");
  }

  SCALP_SCIP_EXC(SCIPcreateConsBasicLinear(scip,&cons,name.c_str(),vars.size(),vars.data(),vals.data(),lhs,rhs));
  SCALP_SCIP_EXC(SCIPaddCons(scip,cons));

  return cons;
}

static bool addConstraint2(SCIP* scip, std::vector<SCIP_CONS*>& constraints, std::map<ILP::Variable,SCIP_VAR*> &variables, ILP::Constraint2 c)
{
  double lhs,rhs;
  ILP::Term& term= c.lhs;

  // map the relations to a <= x+y <= b:
  if(!c.lhs.isConstant() && c.rhs.isConstant() && c.usedRelation== ILP::relation::EQUAL)
  { // x == d
    lhs=c.rhs.constant;
    rhs=c.rhs.constant;
    term=c.lhs;
  }
  else if(c.rhs.isConstant() && !c.lhs.isConstant() && c.usedRelation== ILP::relation::EQUAL)
  { // d == x
    lhs=c.lhs.constant;
    rhs=c.lhs.constant;
    term=c.rhs;
  }
  else if(!c.lhs.isConstant() && c.rhs.isConstant() && c.usedRelation== ILP::relation::LESS_EQ_THAN)
  { // x <= d
    lhs=-ILP::INF();
    rhs=c.rhs.constant;
    term=c.lhs;
  }
  else if(c.rhs.isConstant() && !c.lhs.isConstant() && c.usedRelation== ILP::relation::LESS_EQ_THAN)
  { // d <= x
    lhs=c.lhs.constant;
    rhs=ILP::INF();
    term=c.rhs;
  }
  else if(!c.lhs.isConstant() && c.rhs.isConstant() && c.usedRelation== ILP::relation::MORE_EQ_THAN)
  { // x >= d
    lhs=c.rhs.constant;
    rhs=ILP::INF();
    term=c.lhs;
  }
  else if(!c.rhs.isConstant() && c.lhs.isConstant() && c.usedRelation== ILP::relation::MORE_EQ_THAN)
  { // d >= x
    lhs=-ILP::INF();
    rhs=c.lhs.constant;
    term=c.rhs;
  }
  else
  {
    throw ILP::Exception("Cant add Constraint, most likely this relation is not supported.");
  }

  constraints.push_back(scipAddCons(scip,variables,term,lhs,rhs));

  return true;
}

static bool addConstraint3(SCIP* scip, std::vector<SCIP_CONS*>& constraints, std::map<ILP::Variable,SCIP_VAR*> &variables, ILP::Constraint3 c)
{

  double lhs,rhs;

  if(c.lrel == ILP::relation::LESS_EQ_THAN || c.rrel == ILP::relation::LESS_EQ_THAN)
  { // a <= x <= b
    lhs=c.lbound.constant;
    rhs=c.ubound.constant;
  }
  else if(c.lrel == ILP::relation::MORE_EQ_THAN || c.rrel == ILP::relation::MORE_EQ_THAN)
  { // a >= x >= b
    lhs=c.ubound.constant;
    rhs=c.lbound.constant;
  }
  else
  {
    throw ILP::Exception("This Constraint-Type is not supported at the moment by the SCIP-backend.");
  }
  // TODO: add less- and more-than relations

  constraints.push_back(scipAddCons(scip,variables,c.term,lhs,rhs));

  return true;
}

bool ILP::SolverSCIP::addConstraint(ILP::Constraint c)
{
  if(c.usedType==ILP::Constraint::type::Constraint_2)
  {
    return addConstraint2(scip,constraints,variables,c.c2);
  }
  else if(c.usedType==ILP::Constraint::type::Constraint_3)
  {
    return addConstraint3(scip,constraints,variables,c.c3);
  }
  else
  {
    return false;
  }
}

bool ILP::SolverSCIP::setObjective(ILP::Objective o)
{
  // TODO constant part missing.
  if(o.usedTerm.constant!=0)
  {
    throw ILP::Exception("Constant parts for Objectives are not implemented yet.");
  }

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

  return true;
}

ILP::status ILP::SolverSCIP::solve()
{
  SCALP_SCIP_EXC(SCIPsolve(scip));
  SCIP_SOL* sol = SCIPgetBestSol(scip);

  if(sol!=nullptr)
  {
    for(auto &p:variables)
    {
      res.values.emplace(p.first,SCIPgetSolVal(scip,sol,p.second));
    }
  }

  switch(SCIPgetStatus(scip))
  {
    case SCIP_STATUS_TIMELIMIT: return ILP::status::TIMEOUT;
    case SCIP_STATUS_OPTIMAL: return ILP::status::OPTIMAL;
    case SCIP_STATUS_INFEASIBLE: return ILP::status::INFEASIBLE;
    default: return ILP::status::ERROR;
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
