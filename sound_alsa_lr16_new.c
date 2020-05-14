/*******************************************************************************************
Copyright 2019 H Ueda
Copyright 2019 Mr V Gaydov
(license:GPL)

./sound_alsa_lr16 fs inputfileL inputfileR playtime(sec) device
fs: 44100 etc, device: default OR plughw:0,0 etc,

********************************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <alsa/asoundlib.h>

#define oops(func) (fprintf(stderr, "%s\n", func), exit(1))

static unsigned int nearest_buffer_size(int sample_rate, int n_channels,
                                        unsigned int latency_us,
                                        size_t periods) {
  unsigned int latency_samples =
      (unsigned int)((double)latency_us * sample_rate / 1000000);
  while ((latency_samples * n_channels) % periods != 0) {
    latency_samples++;
  }
  return latency_samples;
}

static void set_hw_params(snd_pcm_t *pcm, snd_pcm_uframes_t *period_size,
                   snd_pcm_uframes_t *buffer_size, snd_pcm_access_t access,
                   snd_pcm_format_t format, int sample_rate, int n_channels,
                   unsigned int latency_us) {
  snd_pcm_hw_params_t *hw_params = NULL;
  snd_pcm_hw_params_alloca(&hw_params);

  // initialize hw_params
  if (snd_pcm_hw_params_any(pcm, hw_params) < 0) {
    oops("snd_pcm_hw_params_any");
  }

  // enable software resampling
  if (snd_pcm_hw_params_set_rate_resample(pcm, hw_params, 1) < 0) {
    oops("snd_pcm_hw_params_set_rate_resample");
  }

  // set number of channels
  if (snd_pcm_hw_params_set_channels(pcm, hw_params, n_channels) < 0) {
    oops("snd_pcm_hw_params_set_channels");
  }

  // set access and format
  int ret;
  if (ret = snd_pcm_hw_params_set_access(pcm, hw_params, access) < 0) {
    oops("snd_pcm_hw_params_set_access");
  }
  if (snd_pcm_hw_params_set_format(pcm, hw_params, format) < 0) {
    oops("snd_pcm_hw_params_set_format");
  }

  // set sample rate
  unsigned int rate = sample_rate;
  if (snd_pcm_hw_params_set_rate_near(pcm, hw_params, &rate, 0) < 0) {
    oops("snd_pcm_hw_params_set_rate_near");
  }
  if (rate != sample_rate) {
    oops("can't set sample rate (exact value is not supported)");
  }

  // calculate nearest buffer size that is multiple of # of periods
  unsigned int n_periods = 4;
  unsigned int suggested_buffer_size =
      nearest_buffer_size(sample_rate, n_channels, latency_us, n_periods);

  // set period size in samples
  // ALSA reads 'period_size' samples from circular buffer every period
  *period_size = suggested_buffer_size / n_periods;
  if (ret = snd_pcm_hw_params_set_period_size_near(pcm, hw_params, period_size,
                                                   NULL) < 0) {
    oops("snd_pcm_hw_params_set_period_size_near");
  }

  // get period time
  unsigned int period_time = 0;
  if (snd_pcm_hw_params_get_period_time(hw_params, &period_time, NULL) < 0) {
    oops("snd_pcm_hw_params_get_period_time");
  }

  // set buffer size, i.e. number of samples in circular buffer
  *buffer_size = *period_size * n_periods;
  if (snd_pcm_hw_params_set_buffer_size_near(pcm, hw_params, buffer_size) < 0) {
    oops("snd_pcm_hw_params_set_buffer_size_near");
  }

  // get buffer time, i.e. total duration of circular buffer in microseconds,
  // calculated from 'sample_rate' and 'buffer_size'
  unsigned int buffer_time = 0;
  if (snd_pcm_hw_params_get_buffer_time(hw_params, &buffer_time, NULL) < 0) {
    oops("snd_pcm_hw_params_get_buffer_time");
  }

  printf("suggested_latency: %u us\n", latency_us);
  printf("suggested_buffer_size: %u samples\n", suggested_buffer_size);
  printf("selected_buffer_time: %u us\n", buffer_time);
  printf("selected_buffer_size: %u samples\n", *buffer_size);
  printf("selected_period_time: %u us\n", period_time);
  printf("selected_period_size: %u samples\n", *period_size);

  // send hw_params to ALSA
  if (snd_pcm_hw_params(pcm, hw_params) < 0) {
    oops("snd_pcm_hw_params");
  }
}

static void set_sw_params(snd_pcm_t *pcm, snd_pcm_uframes_t period_size,
                          snd_pcm_uframes_t buffer_size) {
  snd_pcm_sw_params_t *sw_params = NULL;
  snd_pcm_sw_params_alloca(&sw_params);

  // initialize sw_params
  if (snd_pcm_sw_params_current(pcm, sw_params) < 0) {
    oops("snd_pcm_sw_params_current");
  }

  // set start threshold to buffer_size, so that ALSA starts playback only
  // after circular buffer becomes full first time
  if (snd_pcm_sw_params_set_start_threshold(pcm, sw_params, buffer_size) < 0) {
    oops("snd_pcm_sw_params_set_start_threshold");
  }

  // set minimum number of samples that can be read by ALSA, so that it'll
  // wait until there are at least 'period_size' samples in circular buffer
  if (snd_pcm_sw_params_set_avail_min(pcm, sw_params, period_size) < 0) {
    oops("snd_pcm_sw_params_set_avail_min");
  }

  // send sw_params to ALSA
  if (snd_pcm_sw_params(pcm, sw_params) < 0) {
    oops("snd_pcm_sw_params");
  }
}

int main(int argc, char** argv) {
    if (argc != 6) {
      fprintf(stderr, "usage: %s fs filenameL filenameR playtime device\n",
              argv[0]);
      exit(1);
    }

    // interleaved samples (L R L R ...)
    snd_pcm_access_t access = SND_PCM_ACCESS_RW_INTERLEAVED;

    // 16-bit little-endian signed integers
    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    // stereo (L and R)
    int n_channels = 2;

    // desired latency in microseconds
    unsigned int latency_us = 20000;

    float fs, playtime;
    char filenameL[255], filenameR[255];
    char device[255];

    sscanf(argv[1], "%f", &fs);
    sscanf(argv[2], "%s", filenameL);
    sscanf(argv[3], "%s", filenameR);
    sscanf(argv[4], "%f", &playtime);
    sscanf(argv[5], "%s", device);

    int fsl = (int)(fs * playtime);

    printf("fsl: %f %f %d\n", fs, playtime, fsl);

    FILE *finL = fopen(filenameL, "rb");
    if (finL == NULL) {
        oops("can't open first file");
    }

    FILE *finR = fopen(filenameR, "rb");
    if (finR == NULL) {
        oops("can't open second file");
    }

    printf("file: %s %s\nopened\n", filenameL, filenameR);

    snd_pcm_t* pcm = NULL;
    if (snd_pcm_open(&pcm, device, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        oops("snd_pcm_open");
    }

    snd_pcm_uframes_t period_size = 0, buffer_size = 0;
    set_hw_params(pcm, &period_size, &buffer_size, access, format, (int)fs,
                  n_channels, latency_us);
    set_sw_params(pcm, period_size, buffer_size);

    size_t sample_size = sizeof(int16_t);

    int16_t *buf16l = malloc(sample_size * period_size);
    int16_t *buf16r = malloc(sample_size * period_size);
    int16_t *buf16 = malloc(2 * sample_size * period_size);

    int rptn, n;
    for (rptn = 0; rptn < (int)(fsl / period_size); rptn++) {
      if (fread(buf16l, sizeof(int16_t), period_size, finL) != period_size) {
        break;
      }
      if (fread(buf16r, sizeof(int16_t), period_size, finR) != period_size) {
        break;
      }

      for (n = 0; n < period_size; n++) {
        buf16[2 * n] = buf16l[n];
        buf16[2 * n + 1] = buf16r[n];
      }

      int ret = snd_pcm_writei(pcm, buf16, period_size);
      if (ret < 0) {
        if ((ret = snd_pcm_recover(pcm, ret, 1)) == 0) {
          printf("recovered after xrun (overrun/underrun)\n");
        }
      }

      if (ret < 0) {
        oops("snd_pcm_writei");
      }
    }

    fclose(finL);
    fclose(finR);

    free(buf16l);
    free(buf16r);
    free(buf16);

    snd_pcm_drain(pcm);
    snd_pcm_close(pcm);

    return 0;
}
