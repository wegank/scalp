cmake_minimum_required (VERSION 2.8)
project (ScaLP)

set(CMAKE_MACOSX_RPATH 1)

# Syntax errors are fatal
cmake_policy(SET CMP0010 NEW)

include_directories ("${PROJECT_SOURCE_DIR}/src")

# add the local module path
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# set a standard CMAKE_BUILD_TYPE
#if(NOT_FIRST_RUN)
#else()
#  set(NOT_FIRST_RUN ON CACHE INTERNAL "")
#
#  # set a default build-type
#  set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "The compilation type (Debug,Release,RelWithDebInfo,MinSizeRel)" FORCE)
#endif()
#set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo MinSizeRel "")

# build type specific changes
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#  add_definitions(-D__DEBUG__)
#endif()

# some options and values
#option(USE_GUROBI "Compile the Gurobi-Backend" OFF)
#option(USE_SCIP "Compile the SCIP-Backend" OFF)
#option(USE_CPLEX "Compile the CPLEX-Backend" OFF)
#option(USE_LPSOLVE "Compile the LPSolve-Backend" OFF)
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

#function(checkIncludes nam lis)
#  foreach(I IN LISTS ${lis})
#    if(EXISTS ${I})
#    else()
#      message(SEND_ERROR "${nam}: Can't found the include directory \"${I}\"")
#    endif()
#  endforeach()
#endfunction()
#
#function(checkLibraries nam lis)
#  foreach(I IN LISTS ${lis})
#    string(FIND ${I} "-" TMP)
#    if(${TMP} EQUAL 0)
#    else()
#      if(EXISTS ${I})
#      else()
#        message(SEND_ERROR "${nam}: Can't found the library \"${I}\"")
#      endif()
#    endif()
#  endforeach()
#endfunction()

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
#  mark_as_advanced(FORCE GUROBI_LIBRARIES GUROBI_INCLUDE_DIRS)
  message("Found GUROBI (Header: ${GUROBI_H} ${GUROBI_CPP_H}, Libraries: ${GUROBI_LIB} ${GUROBI_CPP_LIB})")
  set(USE_GUROBI ON CACHE BOOL "Switching GUROBI on")
  include_directories(${GUROBI_H} ${GUROBI_CPP_H})
else(GUROBI_H AND GUROBI_LIB AND GUROBI_CPP_H AND GUROBI_CPP_LIB)
  message("Gurobi was not found and is not used, if you want to use it, pass Gurobi install directory by -DGUROBI_DIR=... or the individual lib and include directories by -DGUROBI_LIBRARIES=... and -DGUROBI_INCLUDE_DIRS=... (Header: ${GUROBI_H} ${GUROBI_CPP_H}, Libraries: ${GUROBI_LIB} ${GUROBI_CPP_LIB})")
#  mark_as_advanced(FORCE GUROBI_LIBRARIES GUROBI_INCLUDE_DIRS)
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
#  mark_as_advanced(FORCE SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
#  include_directories(${SCIP_H})
else(SCIP_H AND SCIP_LIB AND SOPLEX_LIB AND Z_LIB)
  message("SCIP was not found and is not used, if you want to use it, pass SCIP install directory by -DSCIP_DIR=... or the individual lib and include directories by -DSCIP_LIBRARIES=... and -DSCIP_INCLUDE_DIRS=...  (Header: ${SCIP_H}, Libraries: ${SCIP_LIB} ${SOPLEX_LIB} ${Z_LIB})")
#  mark_as_advanced(FORCE SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
endif(SCIP_H AND SCIP_LIB AND SOPLEX_LIB AND Z_LIB)


#find_package(SCIP)
#if(SCIP_FOUND)
#  set(USE_SCIP ON CACHE BOOL "Switch SCIP on")
#  mark_as_advanced(FORCE SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
#  message("Found SCIP, switched it on automatically")
#elseif(USE_SCIP)
#  if((NOT SCIP_LIBRARIES) OR (NOT SCIP_INCLUDE_DIRS))
#    message(SEND_ERROR "SCIP neither found nor set manually. Set SCIP_LIBRARIES and SCIP_INCLUDE_DIRS.")
#  endif()
#  message("Using SCIP libraries and headers located at: ${SCIP_LIBRARIES}, ${SCIP_INCLUDE_DIRS}")
#  checkIncludes("SCIP" SCIP_INCLUDE_DIRS)
#  checkLibraries("SCIP" SCIP_LIBRARIES)
#  mark_as_advanced(CLEAR SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
#elseif(NOT USE_SCIP)
#  message("SCIP was not found and is not used")
#  mark_as_advanced(FORCE SCIP_LIBRARIES SCIP_INCLUDE_DIRS)
#endif()


#find_package(CPLEX QUIET)
#find_package(CPLEX)
#if(CPLEX_FOUND)
#  set(USE_CPLEX ON CACHE BOOL "Switch CPLEX on")
#  mark_as_advanced(FORCE CPLEX_LIBRARIES CPLEX_INCLUDE_DIRS)
#  message("Found CPLEX, switched it on automatically")
#elseif(USE_CPLEX)
#  if((NOT CPLEX_LIBRARIES) OR (NOT CPLEX_INCLUDE_DIRS))
#    message(SEND_ERROR "CPLEX neither found nor set manually. Set CPLEX_LIBRARIES and CPLEX_INCLUDE_DIRS.")
#  endif()
#  message("Using CPLEX libraries and headers located at: ${CPLEX_LIBRARIES}, ${CPLEX_INCLUDE_DIRS}")
#  checkIncludes("CPLEX" CPLEX_INCLUDE_DIRS)
#  checkLibraries("CPLEX" CPLEX_LIBRARIES)
#  mark_as_advanced(CLEAR CPLEX_LIBRARIES CPLEX_INCLUDE_DIRS)
#elseif(NOT USE_CPLEX)
#  message("CPLEX was not found and is not used")
#  mark_as_advanced(FORCE CPLEX_LIBRARIES CPLEX_INCLUDE_DIRS)
#endif()



#
##find_package(LPSOLVE)
#if(LPSOLVE_FOUND)
#  set(USE_LPSOLVE ON CACHE BOOL "Switch lp_solve on")
#  mark_as_advanced(FORCE LPSOLVE_LIBRARIES LPSOLVE_INCLUDE_DIRS)
#  message("Found lp_solve, switched it on automatically")
#elseif(USE_LPSOLVE)
#  if((NOT LPSOLVE_LIBRARIES) OR (NOT LPSOLVE_INCLUDE_DIRS))
#    message(SEND_ERROR "lp_solve neither found nor set manually. Set LPSOLVE_LIBRARIES and LPSOLVE_INCLUDE_DIRS.")
#  endif()
#  message("Using lp_solve libraries and headers located at: ${LPSOLVE_LIBRARIES}, ${LPSOLVE_INCLUDE_DIRS}")
#  checkIncludes("LPSolve" LPSOLVE_INCLUDE_DIRS)
#  checkLibraries("LPSolve" LPSOLVE_LIBRARIES)
#  mark_as_advanced(CLEAR LPSOLVE_LIBRARIES LPSOLVE_INCLUDE_DIRS)
#elseif(NOT USE_LPSOLVE)
#  message("lp_solve was not found and is not used")
#  mark_as_advanced(FORCE LPSOLVE_LIBRARIES LPSOLVE_INCLUDE_DIRS)
#endif()


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


# add backend-libraries

if(USE_GUROBI)
  add_library(ScaLP-Gurobi ${LIBRARY_TYPE}
          src/ScaLP/SolverBackend/SolverGurobi.cpp)
  include_directories(${GUROBI_H} ${GUROBI_CPP_H})
  target_link_libraries(ScaLP-Gurobi ${GUROBI_CPP_LIB} ${GUROBI_LIB} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-Gurobi)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverGurobi.h)
endif()

if(USE_SCIP)
  add_library(ScaLP-SCIP ${LIBRARY_TYPE}
          src/ScaLP/SolverBackend/SolverSCIP.cpp)
  target_include_directories(ScaLP-SCIP PUBLIC ${SCIP_H})
  target_link_libraries(ScaLP-SCIP ${SCIP_LIB} ${SOPLEX_LIB} ${Z_LIB} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-SCIP)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverSCIP.h)
endif()

if(USE_CPLEX)
  add_library(ScaLP-CPLEX ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverCPLEX.cpp)
  target_include_directories(ScaLP-CPLEX PUBLIC ${CPLEX_INCLUDE_DIRS})
  target_link_libraries(ScaLP-CPLEX ${CPLEX_LIBRARIES} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-CPLEX)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverCPLEX.h)
endif()

if(USE_LPSOLVE)
  add_library(ScaLP-LPSolve ${LIBRARY_TYPE}
    src/ScaLP/SolverBackend/SolverLPSolve.cpp)
  target_include_directories(ScaLP-LPSolve PUBLIC ${LPSOLVE_INCLUDE_DIRS})
  target_link_libraries(ScaLP-LPSolve ${LPSOLVE_LIBRARIES} ScaLP)
  list(APPEND BACKEND_TARGETS ScaLP-LPSolve)
  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverLPSolve.h)
endif()

#if(USE_DYNAMIC)
#  add_library(ScaLP-Dynamic ${LIBRARY_TYPE}
#    src/ScaLP/SolverBackend/SolverDynamic.cpp)
#  target_link_libraries(ScaLP-Dynamic dl ScaLP)
#  list(APPEND BACKEND_TARGETS ScaLP-Dynamic)
#  list(APPEND BACKEND_HEADERS src/ScaLP/SolverBackend/SolverDynamic.h)
#endif()
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
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "local installation" FORCE)
endif()

if(${CMAKE_INSTALL_PREFIX} STREQUAL ${PROJECT_SOURCE_DIR})
  add_custom_command(TARGET ScaLP POST_BUILD
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target install)
endif()

install (TARGETS ScaLP ${BACKEND_TARGETS} ${EXECUTABLE_TARGET}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install (FILES ${ScaLP_HEADERS} ${BACKEND_HEADERS}
  DESTINATION include/ScaLP)
