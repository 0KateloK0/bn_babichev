cmake_minimum_required(VERSION 3.20)
project(bn_babichev C)

set(CMAKE_C_STANDARD 11)

add_executable(bn_babichev main.c bn.h bn.c tests.c tests.h)
