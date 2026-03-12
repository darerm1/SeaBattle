cmake_minimum_required(VERSION 3.10)
project(SeaBattle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()
add_subdirectory(server)

find_package(Boost COMPONENTS system QUIET)
if(Boost_FOUND)
    message(STATUS "Boost found: ${Boost_VERSION}")
else()
    message(STATUS "Boost not found, building without Boost (some features may be disabled)")
endif()
