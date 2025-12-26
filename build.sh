#!/usr/bin/env bash

g++ src/main.cpp src/gl.c -Iinclude -lSDL2 -ldl -o demo && ./demo
