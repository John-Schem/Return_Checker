#!/bin/bash

# Build clang plugin
cmake --build build/

if [ "$1" == "c" ];
then
    # Pass plugin to clang and build example c program
    clang -Xclang -load \
      -Xclang build/libReturn_Checker.so -Xclang -add-plugin -Xclang Return_Checker \
       example_programs/source/main.c -o example_programs/example_c.out
elif [ "$1" = "c++" ];
then
    # Pass plugin to clang and build example c++ program
    clang -Xclang -load \
      -Xclang build/libReturn_Checker.so -Xclang -add-plugin -Xclang Return_Checker \
      -lstdc++ -std=c++20 example_programs/source/main.cpp -o example_programs/example_c++.out
else
    echo "Need to provide  c  or  c++  for which example program to use."
    echo "Example: build_and_test_example.sh c or build_and_test_example.sh c++"
    echo "Exiting..."
    exit 22
fi
