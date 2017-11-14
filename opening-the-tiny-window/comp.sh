#!/bin/bash

INPUT=$1

if [[ -n "$INPUT" ]]; then
  INPUT=$1
  OUT=${1/cpp/o}
else
  INPUT="main.cpp glUtils.cpp InputHandler.cpp"
  OUT="main.o"
fi

clang++ -std=c++11 -stdlib=libc++ $INPUT -o $OUT \
  -L /usr/local/lib \
  -lglfw \
  -lsndfile \
  -lao -ldl -lm \
  -framework OpenGL \
  -framework Accelerate
