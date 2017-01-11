cmake_minimum_required(VERSION 2.8.12.2)
project(check-irreducible-cfg)

# CMP0056 is required for any libc++abi dependency of the try_compile() command
# otherwise LD_LIBRARY_DIR needs to be set which is a less than optimal
set(PROJECT_POLICIES CMP0056)

foreach(PROJECT_POLICY ${PROJECT_POLICIES})
  if(POLICY ${PROJECT_POLICY})
    cmake_policy(GET ${PROJECT_POLICY} POLICY_VALUE)
    message("policy ${PROJECT_POLICY}: ${POLICY_VALUE}")
  endif()
endforeach()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -O2")

find_package(LLVM REQUIRED CONFIG)

if(${LLVM_FOUND})
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
else()
    message(FATAL_ERROR "Could not find LLVM")
endif()

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

file(GLOB PROJ_SOURCES src/*.cpp)

add_library(cic MODULE ${PROJ_SOURCES})

llvm_map_components_to_libnames(llvm_libs support core passes)
target_include_directories(cic PUBLIC "include")

target_link_libraries(cic ${llvm_libs})

install(TARGETS cic DESTINATION lib)
install(DIRECTORY "include" DESTINATION .)
