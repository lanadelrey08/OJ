cmake_minimum_required(VERSION 3.14)
project(OJ LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OJ_ENABLE_MYSQL "Build with MySQL Connector/C++ (set MYSQL_CONCPP_INCLUDE / MYSQL_CONCPP_LIB or use vcpkg)" OFF)
option(OJ_ENABLE_REDIS "Build with hiredis" OFF)

set(OJ_SUBPROJECT_BUILD TRUE)

find_package(Threads REQUIRED)

add_subdirectory(common)
add_subdirectory(judge)
add_subdirectory(discussion)
