cmake_minimum_required(VERSION 3.20)
project(pl0 C)

set(CMAKE_C_STANDARD 11)

add_executable(pl0 driver.c codegen.c parser.c lex.c)
