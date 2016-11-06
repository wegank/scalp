
#include <ILP/SolverBackend/SolverSCIP_intern.h>
#include <ILP/SolverBackend/SolverSCIP.h>

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
  SCIP *s;
  SCALP_SCIP_EXC(SCIPcreate(&s));

  return nullptr;
}

ILP::SolverSCIP::SolverSCIP()
{
}

bool ILP::SolverSCIP::addVariable(ILP::Variable v)
{
  (void)(v);
  return false;
}
bool ILP::SolverSCIP::addConstraint(ILP::Constraint cons)
{
  (void)(cons);
  return false;
}
//bool ILP::SolverSCIP::setObjective(ILP::Objective o)
//{
//  (void)(o);
//  return false;
//}
//ILP::status ILP::SolverSCIP::solve()
//{
//  return ILP::status::ERROR;
//}
