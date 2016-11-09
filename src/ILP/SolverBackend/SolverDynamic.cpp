
#include <ILP/SolverBackend/SolverDynamic.h>
#include <ILP/SolverBackend.h>

#include <ILP/Exception.h>

#include <dlfcn.h>  // dlopen, etc.
#include <stdlib.h> // getenv

#include <sstream>

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

ILP::SolverBackend* ILP::newSolverDynamic(std::list<std::string> lsa)
{
  std::list<std::string> ls= getEnvironment();
  ls.splice(ls.end(),lsa);
  ls.unique();

  void *handle;
  ILP::SolverBackend* (*smartconstructor)();

  for(auto& name:ls)
  {
    dlerror(); // free error message
    handle = dlopen(("libILP-"+name+".so").c_str(),RTLD_NOW);
    
    // could not load
    if(handle==nullptr) continue;

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
      return smartconstructor();
    }
  }

  throw ILP::Exception("Could not load any backend");
  return nullptr;
}

ILP::SolverBackend* ILP::newSolverDynamic()
{
  return ILP::newSolverDynamic({});
}
