#pragma once
#include "ScaLP/Solver.h"
#include "ScaLP/Result.h"
#include "ScaLP/Variable.h"
#include <string>

namespace ScaLP
{

bool fileExists(const std::string& s);
bool hasOptimalSolution(const std::string& prefix, const std::string& hash);
bool hasFeasibleSolution(const std::string& prefix, const std::string& hash);
ScaLP::Result getOptimalSolution(const std::string& prefix, const std::string& hash,const ScaLP::VariableSet& vs);
ScaLP::Result getFeasibleSolution(const std::string& prefix, const std::string& hash,const ScaLP::VariableSet& vs);
void writeOptimalSolution(const std::string& prefix, const std::string& hash,ScaLP::Result res,const ScaLP::Solver& solver);
void writeFeasibleSolution(const std::string& prefix, const std::string& hash,ScaLP::Result res,const ScaLP::Solver& solver);
std::pair<bool,double> extractObjective(const std::string& s);

}
