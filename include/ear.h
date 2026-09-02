/* TinyEar-C — optional ears. Honest miss. No ASR. */
#ifndef TINYEAR_EAR_H
#define TINYEAR_EAR_H

#include <stddef.h>
#include <stdint.h>

#define EAR_RATE 16000

typedef struct {
    char manner[16];
    char vowel[12];
    char place[12];
    float f1, f2, zcr_hz, energy;
    int ok;
} ear_class_t;

int ear_read_wav(const char *path, int16_t *out, int cap, int *rate);
float ear_energy(const int16_t *pcm, int n);
float ear_zcr_hz(const int16_t *pcm, int n, int rate);
float ear_goertzel(const int16_t *pcm, int n, int rate, float freq);
void ear_classify(const int16_t *pcm, int n, int rate, ear_class_t *out);

#endif
