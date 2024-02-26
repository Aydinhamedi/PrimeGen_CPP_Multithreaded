cmake_minimum_required(VERSION 3.5.0)
project(PrimeGen_CPP_Multithreaded VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

add_executable(PrimeGen_CPP_Multithreaded Prime_gen.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
