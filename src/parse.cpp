
#include "parse.h"

#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <locale>
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
static const std::string objectiveSection  = "(minimize|maximize|min|max)(.*)";
static const std::string constraintSection = "(subject to|st|s\\.t\\.|st\\.)(.*)";
static const std::string boundariesSection = "(bounds|bound)(.*)";
static const std::string variableSection   = "(binaries|general)(.*)";
static const std::string end               = "(end)";


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

static const std::string spaces = "\\s*";
// a number (x or x.y)
static const std::string number = "(\\d*\\.?\\d*)";
// a variable name
static const std::string variableName = "([a-zA-Z]\\w*)";
// a objective or constraint name
static const std::string name = variableName+":";
// + or -
static const std::string plusMinus = "(\\-|\\+)";

// (coefficient) (variable)
static const std::string unsignedMonomial
  = spaces
  + number // maybe a number
  + spaces
  + "\\*?" // maybe *
  + spaces
  + "(?:"+variableName+"?)"; // maybe an variable

// (+|-) (coefficient) (variable)
static const std::string monomial
  = spaces
  + plusMinus
  + unsignedMonomial;

// (-) (coefficient) (variable)
static const std::string firstMonomial
  = spaces
  + "(\\-?)" // maybe -
  + unsignedMonomial;

// (R)
static const std::string relation
  = spaces
  + "([<>=]=)"
  + spaces;

static std::pair<std::vector<std::string>,std::string> tokenizeTerm(std::string str)
{
  std::smatch m;

  // term-tokens
  std::vector<std::string> ls;

  // tokenize the term
  if(std::regex_search(str,m,std::regex(firstMonomial)))
  {
    std::regex n(monomial);
    do
    {
      for(size_t i=1;i<m.size();++i)
      {
        ls.push_back(m[i]);
      }
      str=m.suffix();

    }
    while(std::regex_search(str,m,n));
  }
  else
  {
    std::cout << "no Term found" << std::endl;
  }

  return {ls,str};
}

static ScaLP::Term parseTerm(std::map<std::string,ScaLP::Variable>& variables, std::vector<std::string>& ls)
{
  ScaLP::Term t;

  if(ls.size()%3==0)
  {
    for(unsigned int i=0;i<ls.size();i+=3)
    {
      std::string sign = ls[i];
      double num = ls[i+1].empty()?1:std::stod(ls[i+1]); // default: 1
      std::string var = ls[i+2];

      if(sign=="-") num = -1*num;

      if(not var.empty())
      { // add coeff*variable
        auto p = variables.emplace(var,ScaLP::newRealVariable(var)); // add as a dummy variable
        t+=num*(p.first->second);
      }
      else
      { // add constant
        t+=num;
      }
    }
  }
  else
  {
    std::cout << "some tokens are missing" << std::endl;
    throw ScaLP::Exception("some tokens are missing");
  }

  return t;
}

static ScaLP::relation parseRelation(std::string s)
{
  if(s=="==") return ScaLP::relation::EQUAL;
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
    std::cout << e.what() << "Could not parse number \"" << lhs << "\" or \"" << rhs << "\"." << std::endl;
    return {false,{}};
  }

  auto p = tokenizeTerm(t);

  if(not p.first.empty())
  {
    ScaLP::Term term = parseTerm(variables,p.first);
    return {true,ScaLP::Constraint(lhs,parseRelation(r1),term,parseRelation(r2),rhs)};
  }
  else
  {
    std::cout << "can't parse constraint3" << std::endl;
    return {false,{}};
  }


}


std::pair<bool,ScaLP::Constraint> parse2(std::map<std::string,ScaLP::Variable>& variables, std::string l, std::string rel, std::string r)
{
  auto tl = tokenizeTerm(l);
  auto ll = parseTerm(variables,tl.first);
  auto re = parseRelation(rel);
  auto tr = tokenizeTerm(r);
  auto rr = parseTerm(variables,tr.first);

  if(rr.isConstant() and ll.isConstant())
  {
    std::cout << "there is no constant part in this constraint:" << std::endl;
    std::cout << "    " << l << " " << rel << " " << r << std::endl;
    return {false,{}};
  }
  else if(ll.isConstant())
  {
    return {true,ScaLP::Constraint(ll.constant,re,rr)};
  }
  else
  {
    return {true,ScaLP::Constraint(ll,re,rr.constant)};

  }

  return {false,{}};
}

static std::pair<bool,ScaLP::Constraint> parseConstraint(std::map<std::string,ScaLP::Variable>& variables, const std::string& str)
{
  std::string term = "";
  std::string rel  = "";
  std::string val  = "";

  std::smatch m;

  std::string reg = "^([^<>=]*)([<>=]=)([^<>]*)([<>]=)?([^<>=]*)$";

  // get the objective name
  if(std::regex_search(str,m,std::regex(reg,std::regex_constants::extended)))
  {
    if(m.size()==6)
    {
      //std::cout << "Constraint tokenized: " << m[0] << std::endl << std::endl;
      if(m[4]=="" and m[5]=="")
      {
        auto r = parse2(variables,m[1],m[2],m[3]);
        return r;
      }
      else if(not (m[1]=="" or m[2]=="" or m[3]=="" or m[4]=="" or m[5]==""))
      {
        auto r = parse3(variables,m[1],m[2],m[3],m[4],m[5]);
        return r;
      }
      else
      {
        std::cout << "something went wrong" << std::endl;
      }
    }
    else
    {
      std::cout << "potential garbage found: " << std::endl;
      std::cout << str << std::endl << std::endl;
    }


  }
  else
  {
    std::cout << "not recognized constraint: " << str << std::endl;
  }

  // TODO: check rest of objstr (p.second) to be valid (empty or comment)

  return {false,{}};
}

static bool parseObjective(std::map<std::string,ScaLP::Variable>& variables, const std::string& s, bool sense, ScaLP::Objective& obj)
{
  std::string objName = "";
  std::string objstr = s;

  std::smatch m;

  // get the objective name
  if(std::regex_search(objstr,m,std::regex(variableName+":")))
  {
    if(m.size()==2)
    {
      objName=m[1];
      objstr=m.suffix();
    }
  }

  // term-tokens
  auto p = tokenizeTerm(objstr);

  // TODO: objective name discarded
  if(sense) obj = ScaLP::maximize(parseTerm(variables,p.first));
  else      obj = ScaLP::minimize(parseTerm(variables,p.first));

  return true;
}

static bool isChar(char c)
{
  return (c>=65 and c<=90) or (c>=97 and c<=122);
}
static bool isDigit(char c)
{
  return c>=48 and c<=57;
}

static bool isVariableName(const std::string& s)
{
  if(s.empty()) return false;
  if(isChar(s.front()))
  {
    return std::all_of(s.begin(),s.end(),[](char c){return isChar(c) or isDigit(c);});
  }
  else
  {
    return false;
  }
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
    const auto it2 = s.find("<=",it+2);
    if(it!=std::string::npos and it2!=std::string::npos)
    {
      lb = std::stod(s.substr(0,it-1));
      ub = std::stod(s.substr(it2+2));
      free = strip(s.substr(it+2,s.size()-it2-2).c_str());
    }
    else
    {
      const auto it = s.find(" = ");
      if(it!=std::string::npos)
      {
        if(isVariableName(s.substr(0,it)))
        {
          free = s.substr(0,it);
          lb = std::stod(s.substr(it+3));
          ub = lb;
        }
        else if(isVariableName(s.substr(it+3)))
        {
          free = s.substr(it+3);
          lb = std::stod(s.substr(0,it));
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
      if(lb!=std::floor(lb) or ub!=std::floor(ub))
      {
        v->second->unsafeSetName(free);
        v->second->unsafeSetType(ScaLP::VariableType::REAL);
        v->second->unsafeSetLowerBound(lb);
        v->second->unsafeSetUpperBound(ub);
      }
      else
      {
        v->second->unsafeSetName(free);
        v->second->unsafeSetType(ScaLP::VariableType::INTEGER);
        v->second->unsafeSetLowerBound(lb);
        v->second->unsafeSetUpperBound(ub);
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
