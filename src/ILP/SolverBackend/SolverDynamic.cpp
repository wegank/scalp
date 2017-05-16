
#include <ILP/SolverBackend/SolverDynamic.h>
#include <ILP/SolverBackend.h>

#include <ILP/Exception.h>
#include <ILP/Result.h>

#include <dlfcn.h>  // dlopen, etc.
#include <stdlib.h> // getenv

#include <sstream>
#include <iostream>

static std::list<std::string> getEnvironment()
{
  std::list<std::string> ls;
  const char* env=getenv("SCALP_SOLVER_LIST");
  if(env!=nullptr)
  {
    std::stringstream s;
    s.str(env);
    std::string entry;
    while(std::getline(s,entry,';'))
    {
      ls.emplace_back(entry);
    }
  }

  return ls;
}

// wrapper-class to manage the library memory
namespace ILP
{
class SolverDynamic : public SolverBackend
{
  public:
  SolverDynamic(std::list<std::string> lsa)
  {
    std::list<std::string> ls= getEnvironment();
    ls.splice(ls.end(),lsa);
    ls.unique();

    void *handle;
    ILP::SolverBackend* (*smartconstructor)();

    for(auto& name:ls)
    {
      // skip empty entries
      if(name.empty()) continue;

      dlerror(); // free error message
      handle = dlopen(("libILP-"+name+".so").c_str(),RTLD_NOW);
      
      // could not load
      if(handle==nullptr)
      {
        std::cerr << dlerror() << std::endl;
        continue;
      }

      // load the constructor
      *(void **) (&smartconstructor) = dlsym(handle, ("newSolver"+name).c_str());

      // error handling
      const char* err = dlerror();
      if(err!=nullptr)
      {
        dlclose(handle);
        continue;
      }
      else
      {
        back=smartconstructor();
        library=handle;
        break;
      }
    }

    if(back==nullptr)
    {
      throw ILP::Exception("Could not load any backend");
    }
    name="Dynamic: "+back->name;
  }

  ~SolverDynamic()
  {
    delete back;
    dlclose(library);
  }

  bool addVariable(const ILP::Variable& v) override
  {
    return back->addVariable(v);
  }
  bool addVariables(ILP::VariableSet vs) override
  {
    return back->addVariables(vs);
  }
  bool addConstraint(const ILP::Constraint& con) override
  {
    return back->addConstraint(con);
  }
  bool addConstraints(std::list<ILP::Constraint> cons) override
  {
    return back->addConstraints(cons);
  }
  bool setObjective(ILP::Objective o) override
  {
    return back->setObjective(o);
  }
  ILP::status solve() override
  {
    ILP::status s = back->solve();
    res = back->res;
    return s;
  }
  void reset() override
  {
    back->reset();
  }
  void setConsoleOutput(bool verbose) override
  {
    back->setConsoleOutput(verbose);
  }
  void setTimeout(long timeout) override
  {
    back->setTimeout(timeout);
  }
  void presolve(bool presolve) override
  {
    back->presolve(presolve);
  }
  void setThreads(unsigned int t) override
  {
    back->setThreads(t);
  }

  private:
  SolverBackend* back;
  void* library;
};

} // namespace ILP

ILP::SolverBackend* ILP::newSolverDynamic(std::list<std::string> lsa)
{
  return dynamic_cast<SolverBackend*>(new SolverDynamic(lsa));
}

ILP::SolverBackend* ILP::newSolverDynamic()
{
  return ILP::newSolverDynamic({});
}
