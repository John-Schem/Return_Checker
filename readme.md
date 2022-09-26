# Return_Checker

Checks to see if return values from functions are being consumed in C and C++. Return_Checker is built as a Clang plugin and passed to clang at compilation. This allows it to integrate into normal development workflow and produce warnings during the compilation process. Will only produce warnings in user-defined code. That is, will not produce warnings in code from header files included in your source files.



## How to Build Plugin

Suggested environment is Ubuntu 22.04

- sudo apt install clang
- sudo apt install cmake
- sudo apt install libclang-dev
- git clone \<repo>
- cd \<repo>
- cmake -B build/
- cmake --build build/

Generated <b>libReturn_Checker.so</b> will be contained in \<repo>/build/ directory

## How to Test with Example Program

- cd \<repo>
- chmod +x build_and_test_example.sh.sh #If needed
- ./build_and_test_example.sh <c or c++>

This will call clang to compile main.cpp in the example program with Return_Checker run as a plugin during the compilation process
