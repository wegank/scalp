cmake_minimum_required (VERSION 3.0)
project (ScaLP)

set(CMAKE_MACOSX_RPATH 1)

# Syntax errors are fatal
cmake_policy(SET CMP0010 NEW)

include_directories ("${PROJECT_SOURCE_DIR}/src")

# some options and values
option(BUILD_SHARED_LIBRARIES "Build as shared libraries" ON)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_SCALP_EXECUTABLE "Build the scalp executable (no functionality yet)" ON)
option(OLD_CXX_ABI "Build with an old C++-abi (activate if you get undefined references with '__cxx11')" OFF)
option(EXPERIMENTAL_PARSER "Build with a experimental lp-parser" OFF)

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

# search for Gurobi:
find_path(GUROBI_H
    NAMES gurobi_c.h
    HINTS ${GUROBI_INCLUDE_DIRS} ${GUROBI_DIR}/include
    PATH_SUFFIXES linux64/include include)

find_library(GUROBI_LIB
    NAMES gurobi70 gurobi75 gurobi80 gurobi81 gurobi90 gurobi91 gurobi95 
    HINTS ${GUROBI_DIR}/lib ${GUROBI_LIBRARIES}
    PATH_SUFFIXES linux64/lib lib)

find_path(GUROBI_CPP_H
        NAMES gurobi_c++.h
        HINTS ${GUROBI_INCLUDE_DIRS} ${GUROBI_DIR}/include
        PATH_SUFFIXES linux64/include include)

find_library(GUROBI_CPP_LIB
    NAMES gurobi_c++
    HINTS ${GUROBI_LIBRARIES} ${GUROBI_DIR}/lib
    PATH_SUFFIXES linux64/lib lib)

IF (GUROBI_H AND GUROBI_LIB AND GUROBI_CPP_H AND GUROBI_CPP_LIB)
  message("Found GUROBI (Header: ${GUROBI_H} ${GUROBI_CPP_H}, Libraries: ${GUROBI_LIB} ${GUROBI_CPP_LIB})")
  set(USE_GUROBI ON CACHE BOOL "Switching GUROBI on")
  include_directories(${GUROBI_H} ${GUROBI_CPP_H})
else(GUROBI_H AND GUROBI_LIB AND GUROBI_CPP_H AND GUROBI_CPP_LIB)
  message("Gurobi was not found and is not used, if you want to use it, pass Gurobi install directory by -DGUROBI_DIR=... or the individual lib and include directories by -DGUROBI_LIBRARIES=... and -DGUROBI_INCLUDE_DIRS=... (Header: ${GUROBI_H} ${GUROBI_CPP_H}, Libraries: ${GUROBI_LIB} ${GUROBI_CPP_LIB})")
endif(GUROBI_H AND GUROBI_LIB AND GUROBI_CPP_H AND GUROBI_CPP_LIB)


# search for SCIP:

find_path(SCIP_H
    NAMES scip/scip.h
    HINTS ${SCIP_INCLUDE_DIRS} ${SCIP_DIR}/include
    PATH_SUFFIXES linux64/include include)

find_library(SCIP_LIB
    NAMES scip
    HINTS ${SCIP_LIBRARIES} ${SCIP_DIR}/lib
    PATH_SUFFIXES linux64/lib lib)

find_library(SOPLEX_LIB
    NAMES soplex
    HINTS ${SCIP_LIBRARIES} ${SCIP_DIR}/lib
    PATH_SUFFIXES linux64/lib lib)

find_library(Z_LIB
    NAMES z
    HINTS ${SCIP_LIBRARIES} ${SCIP_DIR}/lib
    PATH_SUFFIXES linux64/lib lib)


IF (SCIP_H AND SCIP_LIB AND SOPLEX_LIB AND Z_LIB)
  message("Found SCIP (Header: ${SCIP_H}, Libraries: ${SCIP_LIB} ${SOPLEX_LIB} ${Z_LIB})")
  set(USE_SCIP ON CACHE BOOL "Switching SCIP on")
else(SCIP_H AND SCIP_LIB AND SOPLEX_LIB AND Z_LIB)
  message("SCIP was not found and is not used, if you want to use it, pass SCIP install directory by -DSCIP_DIR=... or the individual lib and include directories by -DSCIP_LIBRARIES=... and -DSCIP_INCLUDE_DIRS=...  (Header: ${SCIP_H}, Libraries: ${SCIP_LIB} ${SOPLEX_LIB} ${Z_LIB})")
endif(SCIP_H AND SCIP_LIB AND SOPLEX_LIB AND Z_LIB)

# search for liblpsolve:

find_path(LPSOLVE_H
        NAMES lpsolve/lp_simplex.h
        HINTS ${LPSOLVE_INCLUDE_DIRS} ${LPSOLVE_DIR}/include
        PATH_SUFFIXES linux64/include include)

find_library(LPSOLVE_LIB
        NAMES lpsolve55
        HINTS ${LPSOLVE_LIBRARIES} ${LPSOLVE_DIR}/lib
        PATH_SUFFIXES linux64/lib lib)

if(LPSOLVE_H AND LPSOLVE_LIB)
  message("Found LPSOLVE (Header: ${LPSOLVE_H}, Library: ${LPSOLVE_LIB})")
  set(USE_LPSOLVE ON CACHE BOOL "Switching LPSOLVE on")
else(LPSOLVE_H AND LPSOLVE_LIB)
  message("LPSOLVE was not found and is not used, if you want to use it, pass LPSOLVE install directory by -DLPSOLVE_DIR=... or the individual lib and include directories by -DLPSOLVE_LIBRARIES=... and -DLPSOLVE_INCLUDE_DIRS=...  (Header: ${LPSOLVE_H}, Library: ${LPSOLVE_LIB})")
endif(LPSOLVE_H AND LPSOLVE_LIB)

# search for CPLEX:

find_path(CPLEX_H
        NAMES xxx.h
        HINTS ${CPLEX_INCLUDE_DIRS} ${CPLEX_DIR}/cplex/include/
        PATH_SUFFIXES linux64/include include)

find_path(CONCERT_H
        NAMES xxx.h
        HINTS ${CPLEX_INCLUDE_DIRS} ${CPLEX_DIR}/concert/include/
        PATH_SUFFIXES linux64/include include)

find_library(CPLEX_LIB
        NAMES cplex
        HINTS ${CPLEX_LIBRARIES} ${CPLEX_DIR}/cplex/lib/x86-64_linux/static_pic/
        PATH_SUFFIXES linux64/lib lib)

find_library(ILOCPLEX_LIB
        NAMES ilocplex
        HINTS ${CPLEX_LIBRARIES} ${CPLEX_DIR}/cplex/lib/x86-64_linux/static_pic/
        PATH_SUFFIXES linux64/lib lib)

find_library(CONCERT_LIB
        NAMES concert
        HINTS ${CPLEX_LIBRARIES} ${CPLEX_DIR}/concert/lib/x86-64_linux/static_pic/
        PATH_SUFFIXES linux64/lib lib)

find_library(M_LIB
        NAMES m
        PATH_SUFFIXES linux64/lib lib)

find_library(PTHREAD_LIB
        NAMES pthread
        PATH_SUFFIXES linux64/lib lib)


if(CPLEX_H AND CONCERT_H AND CPLEX_LIB AND ILOCPLEX_LIB AND CONCERT_LIB AND M_LIB AND PTHREAD_LIB)
  message("Found CPLEX (Header: ${CPLEX_H} ${CONCERT_H}, Library: ${CPLEX_LIB} ${ILOCPLEX_LIB} ${CONCERT_LIB} ${M_LIB} ${PTHREAD_LIB})")
  set(USE_CPLEX ON CACHE BOOL "Switching CPLEX on")
else(CPLEX_H AND CONCERT_H AND CPLEX_LIB AND ILOCPLEX_LIB AND CONCERT_LIB AND M_LIB AND PTHREAD_LIB)
  message("CPLEX was not found and is not used, if you want to use it, pass CPLEX install directory by -DCPLEX_DIR=... or the individual lib and include directories by -DCPLEX_LIBRARIES=... and -DCPLEX_INCLUDE_DIRS=... (Header: ${CPLEX_H} ${CONCERT_H}, Library: ${CPLEX_LIB} ${ILOCPLEX_LIB} ${CONCERT_LIB} ${M_LIB} ${PTHREAD_LIB})")
endif(CPLEX_H AND CONCERT_H AND CPLEX_LIB AND ILOCPLEX_LIB AND CONCERT_LIB AND M_LIB AND PTHREAD_LIB)

set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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
  src/ScaLP/SolverBackend/SolverDynamic.h
  src/ScaLP/Utility.h
  )
if(EXPERIMENTAL_PARSER)
  set(PARSER_SOURCES "src/parse.cpp")
else()
  unset(PARSER_SOURCES)
endif()
add_library(ScaLP ${LIBRARY_TYPE}
  src/ScaLP/Constraint.cpp
  src/ScaLP/ResultCache.cpp
  src/ScaLP/Exception.cpp
  src/ScaLP/Objective.cpp
  src/ScaLP/Result.cpp
  src/ScaLP/Solver.cpp
  src/ScaLP/SolverBackend.cpp
  src/ScaLP/Term.cpp
  src/ScaLP/Variable.cpp
  src/ScaLP/SolverBackend/SolverDynamic.cpp
  ${PARSER_SOURCES}
)
target_link_libraries(ScaLP dl)
if(EXPERIMENTAL_PARSER)
  target_compile_definitions(ScaLP PRIVATE "LP_PARSER")
endif()


# add backend-libraries:
if(USE_GUROBI)
  add_library(ScaLP-Gurobi ${LIBRARY_TYPE} src/ScaLP/SolverBackend/SolverGurobi.cpp)
  include_directories(${GUROBI_H} ${GUROBI_CPP_H})
  target_link_libraries(ScaLP-Gurobi ${GUROBI_CPP_LIB} ${GUROBI_LIB} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-Gurobi)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverGurobi.h)
endif()

if(USE_SCIP)
  add_library(ScaLP-SCIP ${LIBRARY_TYPE} src/ScaLP/SolverBackend/SolverSCIP.cpp)
  include_directories(ScaLP-SCIP PUBLIC ${SCIP_H})
  target_link_libraries(ScaLP-SCIP ${SCIP_LIB} ${SOPLEX_LIB} ${Z_LIB} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-SCIP)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverSCIP.h)
endif()

if(USE_CPLEX)
  add_library(ScaLP-CPLEX ${LIBRARY_TYPE} src/ScaLP/SolverBackend/SolverCPLEX.cpp)
  include_directories(ScaLP-CPLEX PUBLIC ${CPLEX_H} ${CONCERT_H})
  target_link_libraries(ScaLP-CPLEX ${CPLEX_LIB} ${ILOCPLEX_LIB} ${CONCERT_LIB} ${M_LIB} ${PTHREAD_LIB} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-CPLEX)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverCPLEX.h)
endif()

if(USE_LPSOLVE)
  add_library(ScaLP-LPSolve ${LIBRARY_TYPE} src/ScaLP/SolverBackend/SolverLPSolve.cpp)
  include_directories(ScaLP-LPSolve PUBLIC ${LPSOLVE_H})
  target_link_libraries(ScaLP-LPSolve ${LPSOLVE_LIB} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-LPSolve)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverLPSolve.h)
endif()

list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverDynamic.h)

if(BUILD_SCALP_EXECUTABLE)
  add_executable(ScaLPBinary
    src/main.cpp
  )
  target_link_libraries(ScaLPBinary ScaLP)
  set_target_properties(ScaLPBinary PROPERTIES INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib")
  set_target_properties(ScaLPBinary PROPERTIES OUTPUT_NAME "scalp")
  set(EXECUTABLE_TARGET ScaLPBinary)
else()
  unset(EXECUTABLE_TARGET)
endif()

if(EXPERIMENTAL_PARSER)
  target_compile_definitions(ScaLPBinary PRIVATE "LP_PARSER")
endif()

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(src/tests)
endif()

# installation
#if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
# set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "local installation" FORCE)
#endif()

#if(${CMAKE_INSTALL_PREFIX} STREQUAL ${PROJECT_SOURCE_DIR})
#  add_custom_command(TARGET ScaLP POST_BUILD
#    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install)
#endif()

install (TARGETS ScaLP ${BACKEND_TARGETS} ${EXECUTABLE_TARGET}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install (FILES ${ScaLP_HEADERS} ${BACKEND_HEADERS}
  DESTINATION include/ScaLP)
