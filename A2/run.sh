#!/bin/sh
gcc -Wall -std=c99 -ansi -o main main.c
gcc -Wall -std=c99 -ansi -o child child.c
./main
