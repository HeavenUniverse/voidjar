/****************************************************************

./pcm_lr infile outfile cutbyte allbyte

cut byte: for example (44 bytes)

****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

FILE *fin, *foutl, *foutr;
char cfoutl[255], cfoutr[255];

int16_t shead[96], spcm[32], sl[16], sr[16];

int main ( int argc, char *argv[] ) {

int ihead, iall;
sscanf( argv[3], "%d", &ihead );
sscanf( argv[3], "%d", &iall );
iall -= ihead;

fin = fopen( argv[1], "rb" );

sprintf( cfoutl, "%s.l", argv[2] );
sprintf( cfoutl, "%s.r", argv[2] );

foutl = fopen( cfoutl, "wb" );
foutr = fopen( cfoutl, "wb" );

fread( shead, ihead, 1, fin );

int i;
for ( i = 0; i < ( iall / 2 ); i++ ) {
fread( &spcm[0], 2, 1, fin );
fwrite( &spcm[0], 1, 1, foutl );
fwrite( &spcm[1], 1, 1, foutr );
}

fclose( fin );
fclose( foutl );
fclose( foutr );


return ( 0 );
}