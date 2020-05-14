/*********************************************************************************
Copyright 2019 H Ueda
(license:LGPL)

option:
command in_file(JPEG) out_file(rgb)

**********************************************************************************/

#include <gtk/gtk.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
GtkWidget 	*window;
GtkWidget 	*canvas;
GdkPixbuf	*pixbuf;
GError **error = malloc( sizeof( GError ) );
int width, height, channel;
gchar *rgb0;

gtk_init (&argc, &argv);

pixbuf = gdk_pixbuf_new_from_file( argv[1], error );
width = gdk_pixbuf_get_width( pixbuf );
height = gdk_pixbuf_get_height( pixbuf );
channel = gdk_pixbuf_get_n_channels( pixbuf );
rgb0 = gdk_pixbuf_get_pixels( pixbuf );
printf( "width:%d\nheight:%d\nchannel:%d\n", width, height, channel );

#define sizeofuchar 3
unsigned char *rgb1, *rgb2;
rgb1 = rgb0;
rgb2 = malloc( (size_t)( width * height ) * sizeofuchar );

int i, j;
int garbage, remainder;

if ( 3 * width %4 == 0 ) {
remainder = 0;
} else {
remainder = 4 - ( 3 * width % 4 );
}

garbage = 0;
for ( i = 0; i < height; i++ ) {
for ( j = 0; j < 3 * width; j++ ) {
 rgb2[ j + ( i * 3 * width ) ] = rgb1[ j + garbage + ( i * 3 * width ) ];
}
garbage += remainder;
}

FILE *fp;
fp = fopen( argv[2], "wb" );
 if ( fp == NULL ) {
  printf( "File not found.\n" );
  exit( 1 );
 }
//fwrite( rgb0, sizeof( gchar ) * width * height, 1, fp );
//!important, not sizeof gchar(might be 1)
fwrite( rgb2, 3 * width * height, 1, fp );
fclose( fp );


return 0;
}
