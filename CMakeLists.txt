cmake_minimum_required(VERSION 3.20)

project(kpgf C CXX)

option(CXX "enable C++ compilation" ON)

if(CXX)
  set(CMAKE_CXX_STANDARD 23)
  enable_language(CXX)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

add_library(combo src/combo.h src/combo.c)

# Visual Studio compiler with static runtime libraries
if(MSVC AND MT)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

set(sources src/main.cpp
        src/Instance.cpp
        src/Instance.h
        src/Params.cpp
        src/Params.h
        src/CompactSolver.cpp
        src/CompactSolver.h
        src/Solution.cpp
        src/Solution.h
        src/CompactSolverResults.cpp
        src/CompactSolverResults.h
        src/ClassCapacitySolver.cpp
        src/ClassCapacitySolver.h
        src/ClassCapacitySolution.cpp
        src/ClassCapacitySolution.h
        src/Column.cpp
        src/Column.h
        src/BranchingRule.cpp
        src/BranchingRule.h
        src/ExtendedSolver.cpp
        src/ExtendedSolver.h
        src/ColumnPool.cpp
        src/ColumnPool.h
        src/ExtendedSolverSolution.cpp
        src/ExtendedSolverSolution.h
        src/BBNode.cpp
        src/BBNode.h
        src/BBNodeSolution.cpp
        src/BBNodeSolution.h
        src/SubProblemSolverGrb.cpp
        src/SubProblemSolverGrb.h
        src/SubProblemSolution.cpp
        src/SubProblemSolution.h
        src/BBTree.cpp
        src/BBTree.h
        src/BPResults.cpp
        src/BPResults.h
        src/FeasibleStatus.h
        src/SubProblemSolverDynPr.cpp
        src/SubProblemSolverDynPr.h
        src/MCKPSolver.cpp
        src/MCKPSolver.h
        src/mcknap.h
        src/mcknap.cpp
        src/MCKPSolverResults.cpp
        src/MCKPSolverResults.h
        src/ClassCardinalityLbSolver.cpp
        src/ClassCardinalityLbSolver.h
        src/ClassCardinalityLbSolution.cpp
        src/ClassCardinalityLbSolution.h
        src/ClassCardinalityUbSolver.cpp
        src/ClassCardinalityUbSolver.h
        src/ClassCardinalityUbSolution.cpp
        src/ClassCardinalityUbSolution.h
        src/CompactSolverRootCallback.cpp
        src/CompactSolverRootCallback.h
)

add_executable(${CMAKE_PROJECT_NAME} ${sources})
set(CMAKE_CXX_STANDARD 23)

if (MSVC)
  target_compile_options(kpgf PRIVATE /W4)
else()
  target_compile_options(kpgf PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/cxxopts)

find_package(GUROBI REQUIRED)
target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PRIVATE ${GUROBI_INCLUDE_DIRS})

include(FetchContent)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE optimized ${GUROBI_CXX_LIBRARY} debug ${GUROBI_CXX_DEBUG_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE ${GUROBI_LIBRARY})
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE combo)