#!/bin/sh
##gcc `pkg-config --cflags gtk+-2.0`   -c -o <file>.o <file>.c
##gcc <file>.o `pkg-config --libs-only-L gtk+-2.0` `pkg-config --libs-only-l gtk+-2.0` -o <file>
gcc `pkg-config --cflags gtk+-2.0`   -c -o "$1.o" "$1.c"
gcc "$1.o" `pkg-config --libs-only-L gtk+-2.0` `pkg-config --libs-only-l gtk+-2.0` -o $1

