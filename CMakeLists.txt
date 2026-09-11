cmake_minimum_required(VERSION 3.16)
project(parallel-computing-cpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

find_package(Threads REQUIRED)

# Boost.Lockfree is the reference the container benchmarks measure against.
# Optional: without it the rest still builds.
find_package(Boost QUIET)

add_library(common INTERFACE)
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(monte_carlo_pi src/monte_carlo_pi.cpp)
target_link_libraries(monte_carlo_pi PRIVATE common Threads::Threads)

if(Boost_FOUND)
  foreach(bench threadsafe_stack_bench threadsafe_queue_bench)
    add_executable(${bench} src/${bench}.cpp)
    target_include_directories(${bench} PRIVATE include ${Boost_INCLUDE_DIRS})
    target_link_libraries(${bench} PRIVATE common Threads::Threads)
  endforeach()
else()
  message(STATUS "Boost not found - skipping the lock-free comparison benchmarks")
endif()
