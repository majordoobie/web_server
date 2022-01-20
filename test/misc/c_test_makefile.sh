#!/bin/bash

gcc -I../../include -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -g -c test_c_subprocess.c -o test_c_subprocess.o

gcc -I../../include -Wall -Wextra -Wpedantic -Waggregate-return -Wwrite-strings -Wvla -Wfloat-equal -g test_c_subprocess.o ../../bin/c_subprocess.o ../../bin/socket_factory.o ../../bin/c_subprocess.a ../../bin/socket_factory.a -o c_subprocess_test

printf "\n\n\nStarting tests...\n"

valgrind -s --leak-check=full --track-origins=yes --show-leak-kinds=all ./c_subprocess_test

rm test_c_subprocess.o c_subprocess_test