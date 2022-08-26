cmake_minimum_required(VERSION 3.22.0)

# Have to include C to get around CMake issues
project(Return_Checker
        VERSION 1.0
        DESCRIPTION "Check those return values!"
        LANGUAGES C CXX)

find_package(Clang REQUIRED CONFIG)

include_directories(SYSTEM "${LLVM_INCLUDE_DIRS};${CLANG_INCLUDE_DIRS}")

add_library(Return_Checker SHARED source/return_checker.cpp)

target_compile_features(Return_Checker PRIVATE cxx_std_20)
target_compile_options(Return_Checker PRIVATE -Wall -Wextra -Wpedantic)
