#!/bin/sh
##gcc <target.c> -lasound -lm -o <target>
gcc "$1.c" -lasound -lm -o $1
