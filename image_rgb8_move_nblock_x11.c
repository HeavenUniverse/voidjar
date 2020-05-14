/************************************************************************************

command filename width height time

*************************************************************************************/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XWDFile.h>
#include <stdio.h>
#include <time.h>

Display *d;
XImage *im;
int width, height, dtime;

time_t timer, prevtimer, crrtimer;

void inputview(char name[]);

int main( int argc, char **argv ) {
Window wr;
GC gc;
Pixmap pr;
XEvent event;

time( &timer );
prevtimer = timer;

sscanf( argv[2], "%d", &width );
sscanf( argv[3], "%d", &height );
sscanf( argv[4], "%d", &dtime );


d=XOpenDisplay(NULL);
wr=XCreateSimpleWindow(d,RootWindow(d,0),10,10,width,height,0,0,0);
XStoreName(d,wr,"voidjar");
XSelectInput(d,wr,ExposureMask);

inputview( argv[1] );
XMapWindow(d,wr);
XMapSubwindows(d,wr);
gc=XCreateGC(d,wr,0,0);
pr=XCreatePixmap(d,wr,width,height,DefaultDepth(d,0));
XSetForeground(d,gc,WhitePixel(d,0));
XFillRectangle(d,pr,gc,0,0,width,height);
XPutImage(d,pr,gc,im,0,0,0,0,width,height);


while( 1 ) {

inputview( argv[1] );
XMapWindow(d,wr);
XMapSubwindows(d,wr);
gc=XCreateGC(d,wr,0,0);
pr=XCreatePixmap(d,wr,width,height,DefaultDepth(d,0));
XSetForeground(d,gc,WhitePixel(d,0));
XFillRectangle(d,pr,gc,0,0,width,height);
XPutImage(d,pr,gc,im,0,0,0,0,width,height);

XNextEvent(d,&event);
switch(event.type) {
 case Expose:
  XCopyArea(d,pr,wr,gc,0,0,width,height,0,0);
  XFlush(d);
 }
}


return( 0 );
}

/*****************************************************************************************/



/*****************************************************************************************/
void inputview( char name[] ) {
XWDFileHeader fh;
FILE *fp;
unsigned char *buff = malloc( 3 * width * height );
int i, flg;
int r0,r1,r2;

if( ( fp = open( name, O_RDONLY | O_NONBLOCK | O_NOFOLLOW ) ) == NULL ) {
 printf( "can not open file!" );
 exit( 1 );
 }
 flg = read( buff, 3, width * height, fp );
 close(fp);

im = XCreateImage(d,DefaultVisual(d,0),DefaultDepth(d,0),ZPixmap,0,0,width,height,32,0);

if( DefaultDepth( d, 0 ) == 16 ) {
 for( i = 0; i < width * height; i++ ) {
  r0=31*buff[3*i+0]/255;
  r1=31*buff[3*i+1]/255;
  r2=31*buff[3*i+2]/255;
  buff[3*i+0]=0x00;
  buff[3*i+1]=r0<<3 | r1>>2;
  buff[3*i+2]=(r1 & 0x07)<<6 | r2;
 }
}

im->data = (char*)buff;
im->xoffset = 0;
im->byte_order = MSBFirst;
im->bitmap_unit = 32;
im->bitmap_bit_order = MSBFirst;
im->bytes_per_line = 3 * width;
im->bits_per_pixel = 24;
}

/*****************************************************************************************/



