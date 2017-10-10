
#include <ScaLP/SolverBackend/SolverLPSolve_intern.h>
#include <ScaLP/SolverBackend/SolverLPSolve.h>

#include <vector>

#include <ScaLP/Exception.h>

#include <iostream>

ScaLP::SolverBackend* newSolverLPSolve()
{
  return new ScaLP::SolverLPSolve();
}

ScaLP::SolverBackend* ScaLP::newSolverLPSolve()
{
  return ::newSolverLPSolve();
}

ScaLP::SolverLPSolve::SolverLPSolve()
  :lp(make_lp(0,0))
{
  set_infinite(lp,ScaLP::INF());
  name="LPSolve";
  this->features.lp=true;
  this->features.ilp=true;
  this->features.qp=false;
  this->features.milp=true;
  this->features.indicators=false;
  this->features.logical=false;
}

ScaLP::SolverLPSolve::~SolverLPSolve()
{
  delete_lp(lp);
}

bool ScaLP::SolverLPSolve::addVariable(const ScaLP::Variable& v)
{
#undef REAL
  bool success=true;
  ++variableCounter;
  success = success && variables.emplace(v,variableCounter).second;
  success = success && set_col_name(lp,variableCounter,const_cast<char*>(v->getName().c_str()));
  success = success && set_bounds(lp,variableCounter,v->getLowerBound(),v->getUpperBound());
  switch (v->getType())
  {
    case ScaLP::VariableType::INTEGER:
      success = success && set_int(lp,variableCounter,true);
      break;
    case ScaLP::VariableType::BINARY:
      success = success && set_binary(lp,variableCounter,true);
      break;
    case ScaLP::VariableType::REAL:
      success = success && set_binary(lp,variableCounter,false);
      success = success && set_int(lp,variableCounter,false);
      // should be the default
      break;
  }
  return success;
}

static char mapRelation(ScaLP::relation r)
{
  switch(r)
  {
    case ScaLP::relation::LESS_EQ_THAN:
      return LE;
    case ScaLP::relation::EQUAL:
      return EQ;
    case ScaLP::relation::MORE_EQ_THAN:
      return GE;
    default:
      throw ScaLP::Exception("Relation (" + ScaLP::Constraint::showRelation(r) + ") not supported at the moment.");
  }
}

// inverts the relation to convert it into normal form.
static ScaLP::relation invertRelation(ScaLP::relation r)
{
  switch(r)
  {
    case ScaLP::relation::LESS_EQ_THAN:
      return ScaLP::relation::MORE_EQ_THAN;
    case ScaLP::relation::MORE_EQ_THAN:
      return ScaLP::relation::LESS_EQ_THAN;
    case ScaLP::relation::EQUAL:
      return ScaLP::relation::EQUAL;
    default:
      throw ScaLP::Exception("Relation (" + ScaLP::Constraint::showRelation(r) + ") not supported at the moment.");
  }
}

bool ScaLP::SolverLPSolve::addConstrH(const ScaLP::Term& t, int rel, double rhs, std::string name)
{
  std::vector<double> coeffs;
  std::vector<int> indices;
  coeffs.reserve(t.sum.size());
  indices.reserve(t.sum.size());

  set_add_rowmode(lp, true);

  for(auto&p:t.sum)
  {
    coeffs.push_back(p.second);
    indices.push_back(variables.at(p.first));
  }

  add_constraintex(lp,coeffs.size(),coeffs.data(),indices.data(),rel,rhs+t.constant);

  set_add_rowmode(lp, false);

  return true;
}

bool ScaLP::SolverLPSolve::addConstraint(const ScaLP::Constraint& cons)
{
  switch(cons.ctype)
  {
    case ScaLP::Constraint::type::C2L:
      addConstrH(cons.term,mapRelation(invertRelation(cons.lrel)),cons.lbound,cons.name);
      break;
    case ScaLP::Constraint::type::C2R:
      addConstrH(cons.term,mapRelation(cons.rrel),cons.ubound,cons.name);
      break;
    case ScaLP::Constraint::type::CEQ:
      addConstrH(cons.term,mapRelation(cons.lrel),cons.lbound,cons.name);
      break;
    case ScaLP::Constraint::type::C3:
      // TODO: better way than two Constraints?
      addConstrH(cons.term,mapRelation(invertRelation(cons.lrel)),cons.lbound,cons.name);
      addConstrH(cons.term,mapRelation(cons.rrel),cons.ubound,cons.name);
      break;
  }

  return true;
}

bool ScaLP::SolverLPSolve::setObjective(ScaLP::Objective o)
{
  set_sense(lp,o.getType()==ScaLP::Objective::type::MAXIMIZE);

  std::vector<double> coeffs;
  std::vector<int> indices;
  coeffs.reserve(o.getTerm().sum.size());
  indices.reserve(o.getTerm().sum.size());

  for(auto&p:o.getTerm().sum)
  {
    coeffs.push_back(p.second);
    indices.push_back(variables.at(p.first));
  }

  objectiveOffset=o.getTerm().constant;

  set_obj_fnex(lp,coeffs.size(),coeffs.data(),indices.data());

  return true;
}

ScaLP::status ScaLP::SolverLPSolve::solve()
{
#undef OPTIMAL
#undef INFEASIBLE
#undef TIMEOUT
  ScaLP::status stat= ScaLP::status::ERROR;
  int resType = ::solve(lp);
  // codes, see: http://lpsolve.sourceforge.net/5.5/solve.htm
  switch(resType)
  {
    case 0: stat = ScaLP::status::OPTIMAL;    break;
    case 1: stat = ScaLP::status::FEASIBLE;   break;
    case 2: stat = ScaLP::status::INFEASIBLE; break;
    case 3: stat = ScaLP::status::UNBOUND;    break;
    case 7: stat = ScaLP::status::TIMEOUT;    break;
  }

  if(stat==ScaLP::status::OPTIMAL or stat==ScaLP::status::FEASIBLE)
  {
    res.objectiveValue=get_objective(lp)+objectiveOffset;

    double* vals;
    get_ptr_variables(lp,&vals);
    for(auto&p:variables)
    {
      res.values.emplace(p.first,vals[p.second-1]);
    }
  }

  return stat;
}

void ScaLP::SolverLPSolve::reset()
{
  // clear the variables-cache
  variables.clear();

  delete_lp(lp);
  lp=make_lp(0,0);
}

void ScaLP::SolverLPSolve::setConsoleOutput(bool verbose)
{
  set_verbose(lp,(verbose)?NORMAL:CRITICAL);
}

void ScaLP::SolverLPSolve::setTimeout(long timeout)
{
  set_timeout(lp,timeout);
}

void ScaLP::SolverLPSolve::presolve(bool presolve)
{
  if(presolve)
  {
    std::cerr << "Scalp: Warning: presolve for lp-solve can lead to a wrong \"all zero\"-result" << std::endl;
    // see: http://lpsolve.sourceforge.net/5.5/set_presolve.htm
    auto all = PRESOLVE_ROWS
      | PRESOLVE_COLS
      | PRESOLVE_LINDEP
      | PRESOLVE_SOS
      | PRESOLVE_REDUCEMIP
      | PRESOLVE_KNAPSACK
      | PRESOLVE_ELIMEQ2
      | PRESOLVE_IMPLIEDFREE
      | PRESOLVE_REDUCEGCD
      | PRESOLVE_PROBEFIX
      | PRESOLVE_PROBEREDUCE
      | PRESOLVE_ROWDOMINATE
      | PRESOLVE_COLDOMINATE
      | PRESOLVE_MERGEROWS
      | PRESOLVE_COLFIXDUAL
      | PRESOLVE_BOUNDS
      | PRESOLVE_DUALS
      | PRESOLVE_SENSDUALS
      ;
    set_presolve(lp,all,get_presolveloops(lp));
  }
  else
  {
    set_presolve(lp,PRESOLVE_NONE,get_presolveloops(lp));
  }
}

void ScaLP::SolverLPSolve::setRelativeMIPGap(double d)
{
  set_mip_gap(lp,false,d);
}

void ScaLP::SolverLPSolve::setAbsoluteMIPGap(double d)
{
  set_mip_gap(lp,true,d);
}
