cmake_minimum_required(VERSION 3.5)

project(RationalWord LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(GMP REQUIRED)

add_executable(RationalWord main.cpp rat64_t.h)
target_link_libraries(RationalWord gmp gmpxx)
