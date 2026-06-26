#include <math.h>
#include <stdlib.h>
#include "game.h"

GameState g_state;

float randf(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

u32 lerpColor(u32 c1, u32 c2, float t) {
    u8 a1 = (c1 >> 24) & 0xFF, b1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, r1 = c1 & 0xFF;
    u8 a2 = (c2 >> 24) & 0xFF, b2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, r2 = c2 & 0xFF;
    u8 a = a1 + (a2 - a1) * t, b = b1 + (b2 - b1) * t, g = g1 + (g2 - g1) * t, r = r1 + (r2 - r1) * t;
    return (a << 24) | (b << 16) | (g << 8) | r;
}

float vecLength(float x, float y) {
    return sqrtf(x * x + y * y);
}

void vecNormalize(float* x, float* y) {
    float len = vecLength(*x, *y);
    if (len > 0.0001f) { *x /= len; *y /= len; }
}
