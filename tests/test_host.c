#include "ear.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int fails = 0;
static void expect(int cond, const char *msg) {
    if (!cond) { fprintf(stderr, "FAIL %s\n", msg); fails++; }
}
static float tone(float hz, float t) {
    return sinf(2.f * (float)M_PI * hz * t);
}

int main(void) {
    int16_t z[1600];
    memset(z, 0, sizeof(z));
    ear_class_t c;
    ear_classify(z, 1600, 16000, &c);
    expect(strcmp(c.manner, "silence") == 0, "silence");
    int16_t s[3200];
    for (int i = 0; i < 3200; i++) {
        float t = (float)i / 16000.f;
        float y = 0.4f * tone(400.f, t) + 0.3f * tone(840.f, t) + 0.3f * tone(1220.f, t);
        if (y > 1.f) y = 1.f;
        if (y < -1.f) y = -1.f;
        s[i] = (int16_t)(y * 20000.f);
    }
    ear_classify(s, 3200, 16000, &c);
    expect(strcmp(c.manner, "vowel") == 0 || strcmp(c.manner, "stop") == 0, "voiced manner");
    expect(c.energy > 0.02f, "energy");
    if (fails) { fprintf(stderr, "%d failures\n", fails); return 1; }
    printf("ok\n");
    return 0;
}
