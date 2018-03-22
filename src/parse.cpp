
#include "parse.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <cmath>

#include "ScaLP/Exception.h"
#include "ScaLP/Solver.h"

static void removeComments(std::string& s)
{
  while(true)
  {
    auto c = s.find('\\');
    if(c==std::string::npos)
    {
      break;
    }
    else
    {
      auto n = s.find('\n',c);
      s.erase(c,n-c);
    }
  }
}

static bool isMax(const std::string& s)
{
  return s[1]=='a' or s[1]=='A';
}
static bool isChar(char c)
{
  return (c>=65 and c<=90) or (c>=97 and c<=122);
}
static bool isDigit(char c)
{
  return c>=48 and c<=57;
}
static bool isSymbol(char c)
{
  std::string s = "!\"#$%&(),.;?@_‘’{}~";
  return s.find(c)!=std::string::npos;
}
static bool isVariableName(const std::string& s)
{
  if(s.empty()) return false;
  if(isChar(s.front()))
  {
    return std::all_of(s.begin()+1,s.end(),[](char c){return isChar(c) or isDigit(c) or isSymbol(c);});
  }
  else
  {
    return false;
  }
}

static std::string strip(const char* line)
{
  std::string stripped(line);
  auto p = stripped.find('\\');
  if(p!=std::string::npos) stripped.erase(p);
  p = stripped.find_first_not_of("\n\t ");
  if(p!=std::string::npos) stripped.erase(0,p);
  p = stripped.find_last_not_of("\n\t ");
  if(p!=std::string::npos) stripped.erase(p+1);

  return stripped;
}

static std::string getprefix(const std::string& line)
{
  std::string r = line.substr(0,16);
  for(char& c:r) c=std::tolower(c);
  return r;
}

static void toLower(std::string& line)
{
  for(char& c:line) c=std::tolower(c);
}

static bool isObjective(const std::string& s)
{
  return s.substr(0,3)=="min" or s.substr(0,3)=="max";
}

static bool isConstraint(const std::string& s)
{
  return s=="subject to" or s=="s.t." or s=="st" or s=="st.";
}

static bool isBounds(const std::string& s)
{
  return s=="bound" or s=="bounds";
}

static bool isBinaries(const std::string& s)
{
  return s=="binary";
}

static std::pair<bool,ScaLP::Term> parseMonominal(std::map<std::string,ScaLP::Variable>& variables, const std::string& s)
{
  auto f = [&variables](const std::string& s) -> std::pair<bool,ScaLP::Term>
  {
    if(isVariableName(s))
    {
      auto var = variables.find(s);
      if(var != variables.end())
      {
        return {true,ScaLP::Term(var->second)};
      }
      else
      {
        ScaLP::Variable v = ScaLP::newIntegerVariable(s);
        variables.emplace(s,v);
        return {true,ScaLP::Term(v)};
      }
    }
    else
    {
      try
      {
        double  c = std::stod(s);
        return {true,ScaLP::Term(c)};
      }
      catch(std::exception e)
      {
        std::cerr << e.what() << std::endl;
      }
    }
  };
  auto it = s.find("*");
  if(it == std::string::npos)
  {
    return f(strip(s.c_str()));
  }
  else
  {
    auto a = f(strip(s.substr(0,it).c_str()));
    auto b = f(strip(s.substr(it+1).c_str()));
    if(a.first and b.first)
    {
      if(a.second.isConstant())
      {
        return {true,a.second.constant*b.second};
      }
      else if(b.second.isConstant())
      {
        return {true,b.second.constant*a.second};
      }
      else
      {
        return {false,{}};
      }
    }
  }
  return {false,{}};
}

static std::pair<bool,ScaLP::Term> parseTerm(std::map<std::string,ScaLP::Variable>& variables, std::string s)
{
  ScaLP::Term t;

  auto it = s.find_first_of("+-");

  if(it==std::string::npos)
  {
    return parseMonominal(variables,s);
  }
  else
  {
    bool lastSign=false; // false + , true -
    if(it==0 and s[it]=='-')
    {
      lastSign=true;
      it = s.find_first_of("+-",1);
    }

    do
    {
      auto p = parseMonominal(variables,strip(s.substr(0,it).c_str()));
      if(p.first)
      {
        if(lastSign) t-= p.second;
        else t+= p.second;
        s = strip(s.substr(it+1).c_str());
        lastSign= s[it]=='-';
        it = s.find_first_of("+-");
      }
      else
      {
        return {false,{}};
      }
    }
    while(it!=std::string::npos);

    // last Monominal
    auto p = parseMonominal(variables,strip(s.c_str()));
    if(not p.first) return {false,{}};
    else
    {
        if(lastSign) t-= p.second;
        else t+= p.second;
    }
    return {true,t};
  }

  return {false,t};
}

static ScaLP::relation parseRelation(std::string s)
{
  if(s=="=") return ScaLP::relation::EQUAL;
  else if(s=="<=") return ScaLP::relation::LESS_EQ_THAN;
  else if(s==">=") return ScaLP::relation::MORE_EQ_THAN;
  else throw ScaLP::Exception("can't parse relation "+s);
}

std::pair<bool,ScaLP::Constraint> parse3(std::map<std::string,ScaLP::Variable>& variables, std::string l, std::string r1, std::string t, std::string r2, std::string r)
{
  double lhs=0;
  double rhs=0;
  try
  {
    lhs = std::stod(l);
    rhs = std::stod(r);
  }
  catch(std::invalid_argument& e)
  {
    std::cerr << e.what() << "Could not parse number \"" << lhs << "\" or \"" << rhs << "\"." << std::endl;
    return {false,{}};
  }
  
  auto p = parseTerm(variables,t);

  if(p.first)
  {
    return {true,ScaLP::Constraint(lhs,parseRelation(r1),p.second,parseRelation(r2),rhs)};
  }
  else
  {
    return {false,{}};
  }

}


std::pair<bool,ScaLP::Constraint> parse2(std::map<std::string,ScaLP::Variable>& variables, std::string l, std::string rel, std::string r)
{
  auto ll = parseTerm(variables,l);
  if(not ll.first) return {false,{}};
  auto re = parseRelation(rel);
  auto rr = parseTerm(variables,r);
  if(not rr.first) return {false,{}};

  if(rr.second.isConstant() and ll.second.isConstant())
  {
    std::cerr << "there is no constant part in this constraint:" << std::endl;
    return {false,{}};
  }
  else if(ll.second.isConstant())
  {
    return {true,ScaLP::Constraint(ll.second.constant,re,rr.second)};
  }
  else if(rr.second.isConstant())
  {
    return {true,ScaLP::Constraint(ll.second,re,rr.second.constant)};
  }

  return {false,{}};
}

static size_t findRelation(const std::string& s, size_t start=0)
{
  auto a = s.find_first_of('=',start);
  if(a==std::string::npos or a==0) return std::string::npos;
  if(s[a-1]=='<' or s[a-1]=='>') return a-1;
  if(s[a+1]=='<' or s[a+1]=='>') return a+1;
  else return a;
}

static std::pair<bool,ScaLP::Constraint> parseConstraint(std::map<std::string,ScaLP::Variable>& variables, const std::string& input)
{
  std::string name="";
  std::string l ="";
  std::string r1 ="";
  std::string term = "";
  std::string r2 ="";
  std::string r ="";
  std::string indicator="";

  std::string str = input;

  {
    size_t collon = str.find(':');
    if(collon!=std::string::npos)
    {
      name= str.substr(0,collon);
      str = str.substr(collon+1);
    }
  }
  {
    size_t arrow = str.find("->");
    if(arrow!=std::string::npos)
    {
      indicator= str.substr(0,arrow);
      str = str.substr(arrow+2);
    }
  }

  size_t r1_pos = findRelation(str);
  size_t r1_len = (str[r1_pos]=='=') ? 1 : 2;
  if(r1_pos!=std::string::npos)
  {
    l  = str.substr(0,r1_pos);
    r1 = str.substr(r1_pos,r1_len);
    size_t r2_pos = findRelation(str,r1_pos+r1_len);
    size_t r2_len = 2; // always 2, because a = t = b is not possible
    if(r2_pos!=std::string::npos)
    {
      term = str.substr(r1_pos+r1_len,r2_pos-r1_pos-r1_len);
      r2 = str.substr(r2_pos,2);
      r  = str.substr(r2_pos+r2_len);
    }
    else
    {
      term = str.substr(r1_pos+r1_len);
    }
  }
  else
  {
    return {false,{}};
  }

  if(r2.empty() and r.empty())
  {
    auto a = parse2(variables,l,r1,term);
    if(a.first and not name.empty()) a.second.setName(name);
    if(not indicator.empty() and a.first)
    {
      auto ind = parseConstraint(variables,indicator);
      return {ind.first,ScaLP::Constraint(ind.second,a.second)};
    }
    return a;
  }
  else
  {
    auto a = parse3(variables,l,r1,term,r2,r);
    if(a.first and not name.empty()) a.second.setName(name);
    if(not indicator.empty() and a.first)
    {
      auto ind = parseConstraint(variables,indicator);
      if(ind.first)
      {
        ind.second.term.sum.begin()->first->unsafeSetType(ScaLP::VariableType::BINARY);
        a.second = ScaLP::Constraint(ind.second,a.second);
      }
    }
    return a;
  }
  return {false,{}};
}

static bool parseObjective(std::map<std::string,ScaLP::Variable>& variables, const std::string& s, bool sense, ScaLP::Objective& obj)
{
  std::string objName = "";
  std::string objstr = s;

  auto it = s.find(':');
  if(it!=std::string::npos and isVariableName(objstr.substr(0,it)))
  {
    objName = objstr.substr(0,it);
  }

  objstr = s.substr(it+1);

  auto p = parseTerm(variables,objstr);

  // TODO: objective name discarded
  if(sense and p.first)
  {
    obj = ScaLP::maximize(p.second);
  }
  else if(p.first)
  {
    obj = ScaLP::minimize(p.second);
  }
  else
  {
    return false;
  }

  return true;
}

static bool parseVariable(std::map<std::string,ScaLP::Variable>& variables, const std::string& s, ScaLP::VariableType t, double lb, double ub)
{
  if(isVariableName(s))
  {
    auto v = variables.find(s);
    if(v!=variables.end())
    {
      v->second->unsafeSetType(t);
    }
    else
    {
      variables.emplace(s,ScaLP::newVariable(s,lb,ub,t));
    }
    return true;
  }
  else
  {
    return false;
  }
}

static bool parseBinaries(std::map<std::string,ScaLP::Variable>& variables, const std::string& s)
{
  return parseVariable(variables,s,ScaLP::VariableType::BINARY,0,1);
}

static bool isGeneral(const std::string& s)
{
  return s=="general";
}

static bool parseGeneral(std::map<std::string,ScaLP::Variable>& variables, const std::string& s)
{
  return parseVariable(variables,s,ScaLP::VariableType::INTEGER,0,ScaLP::INF());
}


static bool parseBoundary(std::map<std::string,ScaLP::Variable>& variables, const std::string& s)
{
  double lb = 0;
  double ub = ScaLP::INF();


  if(s.size() < 5) // need at least "x = 1"
  {
    return false; 
  }
  std::string free = s.substr(s.size()-4);
  toLower(free);

  // set ranges
  if(free!="free")
  {
    const auto it = s.find("<=");
    if(it!=std::string::npos)
    {
      const auto it2 = s.find("<=",it+2);
      if(it2!=std::string::npos)
      {
        lb = std::stod(s.substr(0,it-1));
        ub = std::stod(s.substr(it2+2));
        free = strip(s.substr(it+2,s.size()-it2-2).c_str());
      }
      else
      {

        std::string l = strip(s.substr(0,it-1).c_str());
        std::string r = strip(s.substr(it+2).c_str());

        if(std::all_of(l.begin(),l.end(),[](char c){return isDigit(c) or c=='.';}))
        {
          lb = std::stod(l);
          free = r;
        }
        else if(std::all_of(r.begin(),r.end(),[](char c){return isDigit(c) or c=='.';}))
        {
          ub = std::stod(r);
          free = l;
        }
        else
        {
          return false;
        }
      }
    }
    else
    {
      const auto ite = s.find(" = ");
      if(ite!=std::string::npos)
      {
        if(isVariableName(s.substr(0,ite)))
        {
          free = s.substr(0,ite);
          lb = std::stod(s.substr(ite+3));
          ub = lb;
        }
        else if(isVariableName(s.substr(ite+3)))
        {
          free = s.substr(ite+3);
          lb = std::stod(s.substr(0,ite));
          ub = lb;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
  }
  else
  {
    free = s.substr(0,s.size()-5);
    lb = -ScaLP::INF();
    ub = ScaLP::INF();
  }

  if(isVariableName(free))
  {
    auto v = variables.find(free);
    if(v==variables.end())
    {
      if(lb!=std::floor(lb) or ub!=std::floor(ub))
      {
        variables.emplace(free,ScaLP::newRealVariable(free,lb,ub));
      }
      else
      {
        variables.emplace(free,ScaLP::newIntegerVariable(free,lb,ub));
      }
    }
    else
    {
      v->second->unsafeSetName(free);
      v->second->unsafeSetLowerBound(lb);
      v->second->unsafeSetUpperBound(ub);
      if(lb==0 and ub==1)
      {
        v->second->unsafeSetType(ScaLP::VariableType::BINARY);
      }
      else
      {
        if(v->second->getType()==ScaLP::VariableType::BINARY)
        {
          std::cerr << "Can't extend indicator-variable "
            << free << " to integer or real." << std::endl;
          return false;
        }
        if(lb!=std::floor(lb) or ub!=std::floor(ub))
        {
          v->second->unsafeSetType(ScaLP::VariableType::REAL);
        }
        else
        {
          v->second->unsafeSetType(ScaLP::VariableType::INTEGER);
        }
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

static std::pair<ScaLP::Objective,std::vector<ScaLP::Constraint>> dispatch(std::map<std::string,ScaLP::Variable>& variables, std::ifstream& f)
{
  unsigned int lineNO=0;
  bool objectiveSense = true; // true maximize
  ScaLP::Objective obj;
  std::vector<ScaLP::Constraint> cons;
  enum class section{OBJECTIVE,CONSTRAINT,BOUNDS,BINARIES,GENERAL,NOSECTION};
  section s = section::NOSECTION;
  char line[256]; // lp-format is restricted to 255 characters per line.
  std::string stripped; // striped line
  std::string prefix; // possible section prefix
  std::string acc; // accumulator for multi-line-terms
  while(f.getline(line,256).good())
  {
    ++lineNO;
    stripped = strip(line);
    prefix   = getprefix(stripped);
    if(prefix=="end") break;
    if(prefix.empty()) continue; // empty line
    else if(isObjective(prefix))
    {
      s=section::OBJECTIVE;
      objectiveSense = isMax(prefix);
      continue;
    }
    else if(isConstraint(prefix))
    {
      s=section::CONSTRAINT;
      continue;
    }
    else if(isBounds(prefix))
    {
      s=section::BOUNDS;
      continue;
    }
    else if(isBinaries(prefix))
    {
      s=section::BINARIES;
      continue;
    }
    else if(isGeneral(prefix))
    {
      s=section::GENERAL;
      continue;
    }
    else if(s==section::NOSECTION)
    {
      std::cerr << "Line " << lineNO << " is not in any section" << std::endl;
      break;
    }

    switch(s)
    {
      case section::OBJECTIVE:
        {
          if(stripped.back()=='+' or stripped.back()=='-')
          {
            acc+=stripped;
            break;
          }

          if(not parseObjective(variables,acc+stripped,objectiveSense,obj))
          {
            std::cerr << "Parse error in line " << lineNO << ": This is not a valid objective" << std::endl;
          }
          acc="";
          
          break;
        }
      case section::CONSTRAINT:
        {
          if(stripped.back()=='+' or stripped.back()=='-' or stripped.back()=='=')
          {
            acc+=stripped;
            break;
          }

          auto p = parseConstraint(variables,acc+stripped);
          if(not p.first)
          {
            std::cerr << "Parse error in line " << lineNO << ": This is not a valid constraint" << std::endl;
          }
          else
          {
            cons.push_back(p.second);
          }
          acc="";
          break;
        }
      case section::BOUNDS:
        {
          if(not parseBoundary(variables,stripped))
          {
            std::cerr << "Parse error in line " << lineNO << ": This is not a valid variable boundary" << std::endl;
          }

          break;
        }
      case section::BINARIES:
        {
          if(not parseBinaries(variables,stripped))
          {
            std::cerr << "Parse error in line " << lineNO << ": This is not a valid variable name" << std::endl;
          }
          break;
        }
      case section::GENERAL:
        {
          if(not parseGeneral(variables,stripped))
          {
            std::cerr << "Parse error in line " << lineNO << ": This is not a valid variable name" << std::endl;
          }
          break;
        }
      default:
        {
        }
    }
  }
  return {obj,cons};
}

bool ScaLP::ParserLP::parse(std::string filename,ScaLP::Solver& s)
{
  std::ifstream in(filename);

  std::map<std::string,ScaLP::Variable> variables;

  auto pp = dispatch(variables, in);
  s.setObjective(pp.first);
  for(auto& c:pp.second)
  {
    s << c;
  }

  return true;
}
