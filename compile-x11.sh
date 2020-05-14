#!/bin/sh
##echo compile
##gcc -L/usr/X11R6/lib -o <target> <target source name> -lX11
##To compile, necessary libx11-dev etc
gcc -L/usr/X11R6/lib -o $1 "$1.c" -lX11
