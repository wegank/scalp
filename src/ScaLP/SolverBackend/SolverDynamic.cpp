
#include <ScaLP/SolverBackend/SolverDynamic.h>
#include <ScaLP/SolverBackend.h>

#include <ScaLP/Exception.h>
#include <ScaLP/Result.h>

#include <dlfcn.h>  // dlopen, etc.
#include <stdlib.h> // getenv

#include <sstream>
#include <iostream>

#ifdef __APPLE__
  #define LIBRARY_SUFFIX ".dylib"
#else
  #define LIBRARY_SUFFIX ".so"
#endif

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
namespace ScaLP
{
class SolverDynamic : public SolverBackend
{
  public:
  SolverDynamic(std::list<ScaLP::Feature>fs, std::list<std::string> lsa)
  {
    std::list<std::string> ls= getEnvironment();
    ls.splice(ls.end(),lsa);
    ls.unique();

    void *handle;
    ScaLP::SolverBackend* (*smartconstructor)();

    for(auto& name:ls)
    {
      // skip empty entries
      if(name.empty()) continue;

      dlerror(); // free error message
      handle = dlopen(("libScaLP-"+name+LIBRARY_SUFFIX).c_str(),RTLD_NOW);
      
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
        bool supported=true;
        for(auto f:fs)
        {
          if(not back->featureSupported(f))
          {
            supported=false;
            break;
          }
        }
        if(supported)
        {
          library=handle;
          this->features=back->features;
          break;
        }
        else
        {
          delete back;
          back=nullptr;
          dlclose(handle);
          continue;
        }
      }
    }

    if(back==nullptr)
    {
      throw ScaLP::Exception("Could not load any backend");
    }
    name="Dynamic: "+back->name;
  }

  SolverDynamic(std::list<std::string> lsa)
    :SolverDynamic({},lsa)
  {
  }

  ~SolverDynamic()
  {
    delete back;
    dlclose(library);
  }

  bool addVariable(const ScaLP::Variable& v) override
  {
    return back->addVariable(v);
  }
  bool addVariables(const ScaLP::VariableSet& vs) override
  {
    return back->addVariables(vs);
  }
  bool addConstraint(const ScaLP::Constraint& con) override
  {
    return back->addConstraint(con);
  }
  bool addConstraints(const std::vector<ScaLP::Constraint>& cons) override
  {
    return back->addConstraints(cons);
  }
  bool setObjective(ScaLP::Objective o) override
  {
    return back->setObjective(o);
  }
  std::pair<ScaLP::status,ScaLP::Result> solve() override
  {
    ScaLP::status s;
    ScaLP::Result res;
    std::tie(s,res) = back->solve();
    return {s,res};
  }
  void reset() override
  {
    back->reset();
    objectiveOffset=0;
  }
  void setConsoleOutput(bool verbose) override
  {
    back->setConsoleOutput(verbose);
  }
  void setTimeout(long timeout) override
  {
    back->setTimeout(timeout);
  }
  void setIntFeasTol(double intFeasTol) override
  {
    back->setIntFeasTol(intFeasTol);
  }
  void presolve(bool presolve) override
  {
    back->presolve(presolve);
  }
  void setThreads(unsigned int t) override
  {
    back->setThreads(t);
  }
  void setRelativeMIPGap(double d) override
  {
    back->setRelativeMIPGap(d);
  }
  void setAbsoluteMIPGap(double d) override
  {
    back->setAbsoluteMIPGap(d);
  }

  private:
  SolverBackend* back;
  void* library;
};

} // namespace ScaLP

ScaLP::SolverBackend* ScaLP::newSolverDynamic(std::list<std::string> lsa)
{
  return static_cast<SolverBackend*>(new SolverDynamic(lsa));
}
ScaLP::SolverBackend* ScaLP::newSolverDynamic(std::list<ScaLP::Feature> fs, std::list<std::string> lsa)
{
  return static_cast<SolverBackend*>(new SolverDynamic(fs,lsa));
}

ScaLP::SolverBackend* ScaLP::newSolverDynamic()
{
  return ScaLP::newSolverDynamic({});
}
