/*************************** alsa ******************************

sound_alsa_lr16rec fs filenameL filenameR inputtime(ms) device
device:default OR plughw:0,0

********************************************************************************/
//includes
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>   //int16_t, 
#define ALSA_PCM_NEW_HW_PARAMS_API
#include <alsa/asoundlib.h>


FILE *finL, *finR, *finM;

int channel, bitsize;
float fs, playtime;
char filenameL[255], filenameR[255];
char device[255];

//setting variables
#define BIT8 SND_PCM_FORMAT_S8
#define BIT16 SND_PCM_FORMAT_S16_LE
#define mono 1
#define stereo 2

int erorr = 0;                // result for function return

snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
int mode = 0;
snd_pcm_t *handle;                  // handle for sound_device
snd_pcm_hw_params_t *params;        // params for sound_device
unsigned int sampling_rate;   // sampling rate             //!!1
snd_pcm_uframes_t frames = 32;        // size of frame             //!!2
unsigned int approximate_period;                     // period[us]
unsigned int channels;
snd_pcm_format_t format;
snd_pcm_access_t acc = SND_PCM_ACCESS_RW_INTERLEAVED;
unsigned int soft_resample = 1;
unsigned int latency = 20000;


typedef unsigned int srate;

size_t bytesize = sizeof(int16_t);
size_t buffer_size = 512;
int16_t *buf16, *buf16l, *buf16r;
int16_t swap;

int main(int argc, char **argv) {
sscanf( argv[1], "%f", &fs );
sscanf( argv[2], "%s", filenameL );
sscanf( argv[3], "%s", filenameR );
sscanf( argv[4], "%f", &playtime );
sscanf( argv[5], "%s", device );
format = BIT16;
sampling_rate = (unsigned int)fs;
channels = stereo;

buf16l = malloc( bytesize * buffer_size );
buf16r = malloc( bytesize * buffer_size );
buf16 = malloc( 2 * bytesize * buffer_size );

int fsl;
	fsl = (int)( fs * playtime );

erorr = snd_pcm_open( &handle, device, stream, mode);
if ( erorr != 0 ) {
 printf( "PCM open erorr\n" );
 return( 1 );  
}
erorr = snd_pcm_set_params(handle, format, acc, channels, sampling_rate, soft_resample, latency);
if ( erorr != 0 ) {
 printf( "PCM set erorr\n" );
 return( 1 );  
}

finL = fopen( filenameL, "wb" );
finR = fopen( filenameR, "wb" );

int rptn, n;
for ( rptn = 0; rptn < ( fsl / 512 ); rptn++ ) {

//writei
erorr = snd_pcm_readi( handle, buf16, buffer_size );
if ( erorr < 0 )
 erorr = snd_pcm_recover( handle, erorr, 0 );
if ( erorr < 0 ) {
 printf( "recover failed\n" );
 return( 1 );
 }

 for ( n = 0; n < 256; n++ ) {
  buf16l[n] = buf16[ 2 * n ];
  buf16r[n] = buf16[ 2 * n + 1 ];
 }

 fread( buf16l, sizeof(int16_t), 512, finL );
 fread( buf16r, sizeof(int16_t), 512, finR );

}

fclose( finL );
fclose( finR ); 

snd_pcm_drain(handle);
snd_pcm_close(handle);

return( 0 );
}


