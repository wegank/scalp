#include "ScaLP/Solver.h"
#include "ScaLP/Result.h"
#include "ScaLP/Variable.h"
#include <string>
#include <vector>

namespace ScaLP
{

bool createDirectory(const std::string& d);
bool fileExists(const std::string& s);
std::string hashFNV(const std::string& str);
bool hasOptimalSolution(const std::string& prefix, const std::string& hash);
bool hasFeasibleSolution(const std::string& prefix, const std::string& hash);
ScaLP::Result getOptimalSolution(const std::string& prefix, const std::string& hash,const std::vector<ScaLP::Variable>& vs);
ScaLP::Result getFeasibleSolution(const std::string& prefix, const std::string& hash,const std::vector<ScaLP::Variable>& vs);
void writeOptimalSolution(const std::string& prefix, const std::string& hash,ScaLP::Result res,const ScaLP::Solver& solver);

}
