#!/bin/bash
gcc -std=c11 -pedantic -Wall -Wextra -Werror -fstack-protector-strong -g sokoban.c -o sokoban
valgrind -q ./sokoban < przyklad1.in > a.out && diff a.out przyklad1.out
valgrind -q ./sokoban < przyklad2.in > a.out && diff a.out przyklad2.out
valgrind -q ./sokoban < przyklad3.in > a.out && diff a.out przyklad3.out
rm a.out sokoban
