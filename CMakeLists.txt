cmake_minimum_required (VERSION 2.8)
project (ScaLP)

# Syntax errors are fatal
cmake_policy(SET CMP0010 NEW)

include_directories ("${PROJECT_SOURCE_DIR}/src")

# add the local module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# set a standard CMAKE_BUILD_TYPE
if(NOT_FIRST_RUN)
else()
  set(NOT_FIRST_RUN ON CACHE INTERNAL "")

  # set a default build-type
  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "The compilation type (Debug,Release,RelWithDebInfo,MinSizeRel)" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel "")

# build type specific changes
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#  add_definitions(-D__DEBUG__)
#endif()

# some options and values
option(USE_GUROBI "Compile the Gurobi-Backend" OFF)
option(USE_DYNAMIC "Compile the Dynamic-Backend (GNU/Linux-only at the moment)" ON)
option(USE_SCIP "Compile the SCIP-Backend" OFF)
option(USE_CPLEX "Compile the CPLEX-Backend" OFF)
option(USE_LPSOLVE "Compile the LPSolve-Backend" OFF)
option(BUILD_SHARED_LIBRARIES "Build as shared libraries" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_SCALP_EXECUTABLE "Build the scalp executable (no functionality yet)" ON)
option(OLD_CXX_ABI "Build with an old C++-abi (activate if you get undefined references with '__cxx11')" OFF)

set(CPLEX_LIBRARIES "" CACHE STRING "the paths to the cplex libraries")
set(CPLEX_INCLUDE_DIRS "" CACHE STRING "the paths to the cplex headers")
set(GUROBI_LIBRARIES "" CACHE STRING "the paths to the gurobi libraries")
set(GUROBI_INCLUDE_DIRS "" CACHE STRING "the paths to the gurobi headers")
set(SCIP_LIBRARIES "" CACHE STRING "the paths to the scip libraries")
set(SCIP_INCLUDE_DIRS "" CACHE STRING "the paths to the scip headers")
set(LPSOLVE_LIBRARIES "" CACHE STRING "the paths to the lp_solve libraries")
set(LPSOLVE_INCLUDE_DIRS "" CACHE STRING "the paths to the lp_solve headers")

# hide the paths, maybe they are detected automatically
mark_as_advanced(
  CPLEX_LIBRARIES
  CPLEX_INCLUDE_DIRS
  GUROBI_LIBRARIES
  GUROBI_INCLUDE_DIRS
  SCIP_LIBRARIES
  SCIP_INCLUDE_DIRS
  LPSOLVE_LIBRARIES
  LPSOLVE_INCLUDE_DIRS
  CPLEX_DIR
  GUROBI_DIR
  SCIP_DIR
  LPSOLVE_DIR
  )


# Check if c++11 is supported
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  message(FATAL_ERROR,"The compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

# Deactivate C++11-abi?
if(OLD_CXX_ABI) 
  add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0) 
endif()

if(BUILD_SHARED_LIBRARIES)
  set(LIBRARY_TYPE SHARED)
else()
  set(LIBRARY_TYPE STATIC)
endif()


find_package(CPLEX QUIET)
if(CPLEX_FOUND)
  set(USE_CPLEX ON CACHE BOOL "Switch CPLEX on")
  mark_as_advanced(FORCE CPLEX_LIBRARIES CPLEX_INCLUDE_DIRS)
  message("Found CPLEX, switched it on automatically")
elseif(USE_CPLEX)
  if((NOT CPLEX_LIBRARIES) OR (NOT CPLEX_INCLUDE_DIRS))
    message(SEND_ERROR "CPLEX neither found nor set manually. Set CPLEX_LIBRARIES and CPLEX_INCLUDE_DIRS.")
  endif()
  mark_as_advanced(CLEAR CPLEX_LIBRARIES CPLEX_INCLUDE_DIRS)
elseif(NOT USE_CPLEX)
  mark_as_advanced(FORCE CPLEX_LIBRARIES CPLEX_INCLUDE_DIRS)
endif()

find_package(GUROBI QUIET)
if(GUROBI_FOUND)
  set(USE_GUROBI ON CACHE BOOL "Switch GUROBI on")
  mark_as_advanced(FORCE GUROBI_LIBRARIES GUROBI_INCLUDE_DIRS)
  message("Found GUROBI, switched it on automatically")
elseif(USE_GUROBI)
  if((NOT GUROBI_LIBRARIES) OR (NOT GUROBI_INCLUDE_DIRS))
    message(SEND_ERROR "Gurobi neither found nor set manually. Set GUROBI_LIBRARIES and GUROBI_INCLUDE_DIRS.")
  endif()
  mark_as_advanced(CLEAR GUROBI_LIBRARIES GUROBI_INCLUDE_DIRS)
elseif(NOT USE_GUROBI)
  mark_as_advanced(FORCE GUROBI_LIBRARIES GUROBI_INCLUDE_DIRS)
endif()

find_package(SCIP QUIET)
if(SCIP_FOUND)
  set(USE_SCIP ON CACHE BOOL "Switch SCIP on")
  mark_as_advanced(FORCE SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
  message("Found SCIP, switched it on automatically")
elseif(USE_SCIP)
  if((NOT SCIP_LIBRARIES) OR (NOT SCIP_INCLUDE_DIRS))
    message(SEND_ERROR "SCIP neither found nor set manually. Set SCIP_LIBRARIES and SCIP_INCLUDE_DIRS.")
  endif()
  mark_as_advanced(CLEAR SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
elseif(NOT USE_SCIP)
  mark_as_advanced(FORCE SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
endif()

find_package(LPSOLVE QUIET)
if(LPSOLVE_FOUND)
  set(USE_LPSOLVE ON CACHE BOOL "Switch lp_solve on")
  mark_as_advanced(FORCE LPSOLVE_LIBRARIES LPSOLVE_INCLUDE_DIRS)
  message("Found lp_solve, switched it on automatically")
elseif(USE_LPSOLVE)
  if((NOT LPSOLVE_LIBRARIES) OR (NOT LPSOLVE_INCLUDE_DIRS))
    message(SEND_ERROR "lp_solve neither found nor set manually. Set LPSOLVE_LIBRARIES and LPSOLVE_INCLUDE_DIRS.")
  endif()
  mark_as_advanced(CLEAR LPSOLVE_LIBRARIES LPSOLVE_INCLUDE_DIRS)
elseif(NOT USE_LPSOLVE)
  mark_as_advanced(FORCE LPSOLVE_LIBRARIES LPSOLVE_INCLUDE_DIRS)
endif()

# add main library

set(ScaLP_HEADERS
  src/ScaLP/Constraint.h
  src/ScaLP/Exception.h
  src/ScaLP/Objective.h
  src/ScaLP/Result.h
  src/ScaLP/Solver.h
  src/ScaLP/SolverBackend.h
  src/ScaLP/Term.h
  src/ScaLP/Variable.h
  )
add_library(ScaLP ${LIBRARY_TYPE}
  src/ScaLP/Constraint.cpp
  src/ScaLP/Exception.cpp
  src/ScaLP/Objective.cpp
  src/ScaLP/Result.cpp
  src/ScaLP/Solver.cpp
  src/ScaLP/SolverBackend.cpp
  src/ScaLP/Term.cpp
  src/ScaLP/Variable.cpp
)

# add backend-libraries

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

if(USE_CPLEX)
  include_directories(${CPLEX_INCLUDE_DIRS})
  add_library(ScaLP-CPLEX ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverCPLEX.cpp)
  target_link_libraries(ScaLP-CPLEX ${CPLEX_LIBRARIES} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-CPLEX)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverCPLEX.h)
endif()

if(USE_GUROBI)
  include_directories(${GUROBI_INCLUDE_DIRS})
  add_library(ScaLP-Gurobi ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverGurobi.cpp)
  target_link_libraries(ScaLP-Gurobi ${GUROBI_LIBRARIES} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-Gurobi)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverGurobi.h)
endif()

if(USE_SCIP)
  include_directories(${SCIP_INCLUDE_DIRS})
  add_library(ScaLP-SCIP ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverSCIP.cpp)
  target_link_libraries(ScaLP-SCIP ${SCIP_LIBRARIES} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-SCIP)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverSCIP.h)
endif()

if(USE_LPSOLVE)
  include_directories(${LPSOLVE_INCLUDE_DIRS})
  add_library(ScaLP-LPSolve ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverLPSolve.cpp)
  target_link_libraries(ScaLP-LPSolve ${LPSOLVE_LIBRARIES} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-LPSolve)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverLPSolve.h)
endif()

if(USE_DYNAMIC)
  add_library(ScaLP-Dynamic ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverDynamic.cpp)
  target_link_libraries(ScaLP-Dynamic dl ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-Dynamic)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverDynamic.h)
endif()



if(BUILD_SCALP_EXECUTABLE)
  add_executable(scalp
    src/main.cpp
  )
  target_link_libraries(scalp ScaLP ScaLP-Dynamic)
  set_target_properties(scalp PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(src/tests)
endif()

# installation
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "local installation" FORCE)
endif()

install (TARGETS ScaLP ${BACKEND_TARGETS} scalp
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
#INCLUDES DESTINATION include)
install (FILES ${ScaLP_HEADERS} ${BACKEND_HEADERS}
  DESTINATION include/ScaLP)
