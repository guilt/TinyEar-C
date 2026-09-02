#include "ear.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float ear_energy(const int16_t *pcm, int n) {
    if (n <= 0) return 0.f;
    double acc = 0.0;
    for (int i = 0; i < n; i++) acc += (double)pcm[i] * (double)pcm[i];
    float rms = (float)sqrt(acc / (double)n);
    float e = rms / 16000.f;
    return e > 1.f ? 1.f : e;
}

float ear_zcr_hz(const int16_t *pcm, int n, int rate) {
    if (n < 2) return 0.f;
    int zc = 0;
    for (int i = 1; i < n; i++) {
        int a = pcm[i - 1] >= 0;
        int b = pcm[i] >= 0;
        if (a != b) zc++;
    }
    return 0.5f * (float)zc * (float)rate / (float)n;
}

float ear_goertzel(const int16_t *pcm, int n, int rate, float freq) {
    if (n <= 0) return 0.f;
    float w = (float)(2.0 * M_PI * freq / (float)rate);
    float coeff = 2.f * cosf(w);
    float s0 = 0, s1 = 0, s2 = 0;
    const float scale = 1.f / 32768.f;
    for (int i = 0; i < n; i++) {
        s0 = pcm[i] * scale + coeff * s1 - s2;
        s2 = s1;
        s1 = s0;
    }
    return s1 * s1 + s2 * s2 - coeff * s1 * s2;
}

static float peak_bin(const int16_t *pcm, int n, int rate, const float *bins, int nb) {
    float best_f = bins[0], best_p = -1.f;
    for (int i = 0; i < nb; i++) {
        float p = ear_goertzel(pcm, n, rate, bins[i]);
        if (p > best_p) { best_p = p; best_f = bins[i]; }
    }
    return best_f;
}

static void nearest_vowel(float f1, float f2, char *out, size_t cap) {
    struct { const char *n; float f1, f2; } tab[] = {
        {"i", 310, 2565}, {"e", 610, 2061}, {"a", 840, 1221},
        {"o", 655, 941}, {"u", 345, 974}, {"schwa", 575, 1680},
    };
    float best = 1e18f;
    const char *name = "schwa";
    for (size_t i = 0; i < sizeof(tab) / sizeof(tab[0]); i++) {
        float d1 = (f1 - tab[i].f1) / 400.f;
        float d2 = (f2 - tab[i].f2) / 800.f;
        float d = d1 * d1 + d2 * d2;
        if (d < best) { best = d; name = tab[i].n; }
    }
    snprintf(out, cap, "%s", name);
}

void ear_classify(const int16_t *pcm, int n, int rate, ear_class_t *out) {
    memset(out, 0, sizeof(*out));
    float e = ear_energy(pcm, n);
    float z = ear_zcr_hz(pcm, n, rate);
    out->energy = e;
    out->zcr_hz = z;
    if (e < 0.012f) {
        snprintf(out->manner, sizeof(out->manner), "silence");
        snprintf(out->place, sizeof(out->place), "none");
        out->ok = 1;
        return;
    }
    static const float f1b[] = {250, 350, 450, 550, 650, 750, 850, 950, 1100};
    static const float f2b[] = {800, 950, 1100, 1300, 1500, 1700, 1900, 2100, 2300, 2600};
    float f1 = peak_bin(pcm, n, rate, f1b, 9);
    float f2 = peak_bin(pcm, n, rate, f2b, 10);
    out->f1 = f1;
    out->f2 = f2;
    float low = (ear_goertzel(pcm, n, rate, 200) + ear_goertzel(pcm, n, rate, 550) +
                 ear_goertzel(pcm, n, rate, 900)) / 3.f;
    float high = (ear_goertzel(pcm, n, rate, 3000) + ear_goertzel(pcm, n, rate, 4500) +
                  ear_goertzel(pcm, n, rate, 6000)) / 3.f;
    float tilt = high / (low > 1e-9f ? low : 1e-9f);
    const char *place = f2 >= 1600.f ? "coronal" : (f2 < 1100.f ? "labial" : "velar");
    snprintf(out->place, sizeof(out->place), "%s", place);
    if (tilt >= 2.2f) {
        snprintf(out->manner, sizeof(out->manner), z >= 1400.f ? "fricative" : "stop");
        out->ok = 1;
        return;
    }
    nearest_vowel(f1, f2, out->vowel, sizeof(out->vowel));
    snprintf(out->manner, sizeof(out->manner), "vowel");
    snprintf(out->place, sizeof(out->place), "vowel");
    out->ok = 1;
}

static unsigned int u32le(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8) |
           ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}
static unsigned int u16le(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

int ear_read_wav(const char *path, int16_t *out, int cap, int *rate) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return -1;
    unsigned char hdr[44];
    if (fread(hdr, 1, 44, fp) != 44) { fclose(fp); return -1; }
    if (memcmp(hdr, "RIFF", 4) != 0 || memcmp(hdr + 8, "WAVE", 4) != 0) {
        fclose(fp); return -1;
    }
    *rate = (int)u32le(hdr + 24);
    unsigned int bits = u16le(hdr + 34);
    unsigned int data_bytes = u32le(hdr + 40);
    if (bits != 16) { fclose(fp); return -1; }
    int n = (int)(data_bytes / 2u);
    if (n > cap) n = cap;
    if (fread(out, 2, (size_t)n, fp) != (size_t)n) { fclose(fp); return -1; }
    fclose(fp);
    return n;
}
