#!/bin/sh
clear
gcc -Wall -std=c99 -o main main.c
gcc -Wall -std=c99 -o counter counter.c

./counter 5
./main
