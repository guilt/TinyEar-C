#include "ear.h"

#include <stdio.h>
#include <stdlib.h>

#define CAP (EAR_RATE * 4)

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: ear <file.wav>\n");
        return 2;
    }
    int16_t *buf = (int16_t *)malloc((size_t)CAP * 2);
    if (!buf) return 2;
    int rate = 0;
    int n = ear_read_wav(argv[1], buf, CAP, &rate);
    if (n < 0) {
        fprintf(stderr, "cannot read %s\n", argv[1]);
        free(buf);
        return 1;
    }
    ear_class_t c;
    ear_classify(buf, n, rate, &c);
    printf("file\t%s\n", argv[1]);
    printf("samples\t%d\n", n);
    printf("rate\t%d\n", rate);
    printf("manner\t%s\n", c.manner);
    printf("vowel\t%s\n", c.vowel);
    printf("place\t%s\n", c.place);
    printf("f1\t%.0f\n", c.f1);
    printf("f2\t%.0f\n", c.f2);
    printf("zcr\t%.1f\n", c.zcr_hz);
    printf("energy\t%.3f\n", c.energy);
    printf("ok\t%s\n", c.ok ? "true" : "false");
    printf("belief\tI did not catch words.\n");
    free(buf);
    return 0;
}
