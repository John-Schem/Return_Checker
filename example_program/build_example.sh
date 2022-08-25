#!/bin/bash

clang -Xclang -load \
      -Xclang ../build/libReturn_Checker.so -Xclang -add-plugin -Xclang Return_Checker \
      -lstdc++ -std=c++20 source/main.cpp