#include <math.h>
#include <stdlib.h>
#include "starfield.h"

// ---------------------------------------------------------------
// starfield.c
// Génère un champ d'étoiles dans une zone "monde" autour du point
// de départ. Les étoiles sont positionnées en coordonnées MONDE et
// projetées à l'écran via la caméra (g_state.camX/camY), ce qui
// donne un parallax simple et cohérent avec le déplacement du
// vaisseau. Au-delà d'une certaine vitesse, on dessine une traînée
// (ligne) au lieu d'un simple point => effet d'hypervitesse.
// ---------------------------------------------------------------

#define STAR_FIELD_RANGE 3000.0f   // étendue du champ d'étoiles autour du joueur
#define HYPERSPEED_THRESHOLD 140.0f // vitesse (px/s monde) au-delà de laquelle on étire les étoiles

static const u32 STAR_COLORS[] = {
    0xFFFFFFFF, // blanc
    0xFFAEEFFF, // bleuté
    0xFF9FD9FF, // bleu clair
    0xFFCFE8FF, // blanc bleuté
    0xFFFFE9B0, // jaune pâle
    0xFFFFD27F, // orange pâle
};

void starfield_init(void) {
    for (int i = 0; i < MAX_STARS; i++) {
        Star* s = &g_state.stars[i];
        s->worldX = randf(-STAR_FIELD_RANGE, STAR_FIELD_RANGE);
        s->worldY = randf(-STAR_FIELD_RANGE, STAR_FIELD_RANGE);
        s->size = randf(1.0f, 2.6f);
        s->color = STAR_COLORS[rand() % (sizeof(STAR_COLORS) / sizeof(u32))];
        s->twinkles = (rand() % 100) < 40; // ~40% des étoiles scintillent
        s->twinkleSpeed = randf(1.5f, 4.0f);
        s->brightness = randf(0.5f, 1.0f);
    }
}

// Réinjecte une étoile de l'autre côté du champ si elle sort trop loin
// de la caméra, pour donner l'illusion d'un espace infini sans avoir
// à gérer des milliers d'étoiles à la fois.
static void wrapStarIfNeeded(Star* s) {
    float dx = s->worldX - g_state.camX;
    float dy = s->worldY - g_state.camY;
    if (dx > STAR_FIELD_RANGE)  s->worldX -= STAR_FIELD_RANGE * 2;
    if (dx < -STAR_FIELD_RANGE) s->worldX += STAR_FIELD_RANGE * 2;
    if (dy > STAR_FIELD_RANGE)  s->worldY -= STAR_FIELD_RANGE * 2;
    if (dy < -STAR_FIELD_RANGE) s->worldY += STAR_FIELD_RANGE * 2;
}

void starfield_update(float dt, float shipSpeed) {
    (void)shipSpeed;
    for (int i = 0; i < MAX_STARS; i++) {
        Star* s = &g_state.stars[i];
        if (s->twinkles) {
            s->brightness = 0.55f + 0.45f * sinf(g_state.frameCount * 0.02f * s->twinkleSpeed + i);
        }
        wrapStarIfNeeded(s);
    }
}

static inline u32 colorWithAlpha(u32 color, float alphaMul) {
    u8 a = (u8)(((color >> 24) & 0xFF) * alphaMul);
    return (color & 0x00FFFFFF) | (a << 24);
}

void starfield_render(C2D_TextBuf textBuf) {
    (void)textBuf;
    float speed = vecLength(g_state.ship.vx, g_state.ship.vy);
    bool hyperspeed = speed > HYPERSPEED_THRESHOLD;

    for (int i = 0; i < MAX_STARS; i++) {
        Star* s = &g_state.stars[i];
        float sx = s->worldX - g_state.camX + TOP_W / 2.0f;
        float sy = s->worldY - g_state.camY + TOP_H / 2.0f;

        if (sx < -20 || sx > TOP_W + 20 || sy < -20 || sy > TOP_H + 20) continue;

        u32 col = colorWithAlpha(s->color, s->brightness);

        if (hyperspeed) {
            // Étire l'étoile dans la direction opposée au déplacement
            float stretch = fminf((speed - HYPERSPEED_THRESHOLD) * 0.05f, 18.0f);
            float dirX = -g_state.ship.vx / (speed + 0.001f);
            float dirY = -g_state.ship.vy / (speed + 0.001f);
            C2D_DrawLine(sx, sy, col, sx + dirX * stretch, sy + dirY * stretch,
                         col & 0x00FFFFFF, s->size * 0.7f, 0.0f);
        } else {
            // halo discret : un petit disque semi-transparent + le point lumineux
            C2D_DrawCircleSolid(sx, sy, 0.0f, s->size * 2.2f, colorWithAlpha(s->color, s->brightness * 0.18f));
            C2D_DrawCircleSolid(sx, sy, 0.0f, s->size, col);
        }
    }
}
