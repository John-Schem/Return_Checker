# Return_Checker

Checks to see if return values from functions are being consumed. Return_Checker is built as a Clang plugin and passed to clang at compilation. This allows it to integrate into normal development workflow and produce warnings during the compilation process.



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

## How to Test with Example Plugin

Make sure to build the plugin first

- cd \<repo>
- chmod +x build_example_program.sh #If needed
- ./build_example_program.sh

This will call clang to compile main.cpp in the example program with Return_Checker run as a plugin during the compilation process
