/*********************************************************************************
Copyright 2019 H Ueda
(license:LGPL)

option:
image_rgb8_gtk file_name width height time(s)

**********************************************************************************/

#include <gtk/gtk.h>
#include <stdlib.h>

#include <time.h>

int playtime;
time_t timer, crrtimer;

static gboolean timeout_exit( void ) {                  //move
if ( time( &crrtimer ) - timer >= (time_t)playtime ) {
	exit( 0 );
}
return TRUE;
}

static gint cb_expose (GtkWidget	*widget,
		       GdkEventExpose	*event,
		       gpointer		data) {
  gdk_draw_pixbuf (widget->window, NULL, (GdkPixbuf *) data,
		   0, 0, 0, 0, -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
  return TRUE;
}
     
int main (int argc, char *argv[]) {
GtkWidget 	*window;
GtkWidget 	*canvas;
GdkPixbuf	*pixbuf;
int width, height;

time( &timer );
sscanf( argv[4], "%d", &playtime );

gtk_init (&argc, &argv);

sscanf( argv[2], "%d", &width );
sscanf( argv[3], "%d", &height );
unsigned char *rgb = malloc( 3*width*height );

FILE *fp = fopen( argv[1], "rb" );
fread( rgb, 3*width*height, 1, fp );
fclose( fp );

pixbuf = gdk_pixbuf_new_from_data( rgb, GDK_COLORSPACE_RGB, 
FALSE, 8, width, height, ( 3 * width ), NULL, NULL );

  
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(window), "voidjar");  
  g_signal_connect (G_OBJECT(window), "destroy",
		    G_CALLBACK(gtk_main_quit), NULL);

  canvas = gtk_drawing_area_new ();
  gtk_widget_set_size_request (canvas,
			       gdk_pixbuf_get_width (pixbuf),
			       gdk_pixbuf_get_height(pixbuf));
  g_signal_connect (G_OBJECT(canvas), "expose_event",
		    G_CALLBACK(cb_expose), pixbuf);
  gtk_container_add (GTK_CONTAINER(window), canvas);

gtk_timeout_add( 500, (GtkFunction)timeout_exit, NULL );
  
  gtk_widget_show_all (window);
  gtk_main ();

  return 1;
}
