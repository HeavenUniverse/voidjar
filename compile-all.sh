#!/bin/sh
gcc "sound_alsa_lr16_new.c" -lasound -lm -o sound_alsa_lr16
gcc `pkg-config --cflags gtk+-2.0`   -c -o "image_rgb8_gtk.o" "image_rgb8_gtk.c"
gcc "image_rgb8_gtk.o" `pkg-config --libs-only-L gtk+-2.0` `pkg-config --libs-only-l gtk+-2.0` -o image_rgb8_gtk
gcc `pkg-config --cflags gtk+-2.0`   -c -o "jpeg_rgb_gtk.o" "jpeg_rgb_gtk.c"
gcc "jpeg_rgb_gtk.o" `pkg-config --libs-only-L gtk+-2.0` `pkg-config --libs-only-l gtk+-2.0` -o jpeg_rgb_gtk
gcc -L/usr/X11R6/lib -o image_rgb8_x11 "image_rgb8_x11.c" -lX11

