#include <stdio.h>
#include <math.h>
#include "bottomscreen.h"

// ---------------------------------------------------------------
// bottomscreen.c
// Écran tactile du bas : par défaut affiche une mini-map vue de
// dessus du système (vaisseau + planètes) avec une icône
// "engrenage" en haut à droite. Toucher l'icône ouvre un panneau
// de réglages (couleur de l'UI, musique on/off, piste suivante).
// ---------------------------------------------------------------

#define GEAR_X 280
#define GEAR_Y 8
#define GEAR_SIZE 32

#define MAP_SCALE 0.12f // ratio monde -> pixels mini-map

static const u32 ACCENT_PRESETS[] = {
    0xFF6FE0FF, // cyan (défaut)
    0xFF6FFFA0, // vert
    0xFFFFC85C, // orange
    0xFFB07CFF, // violet
    0xFF5C8CFF, // bleu
};
#define ACCENT_COUNT (sizeof(ACCENT_PRESETS) / sizeof(u32))

static C2D_TextBuf s_buf;
static bool s_wasTouching = false;

void bottomscreen_init(C2D_TextBuf* textBuf) {
    *textBuf = C2D_TextBufNew(1024);
    s_buf = *textBuf;
    g_state.uiAccentColor = ACCENT_PRESETS[0];
    g_state.musicOn = true;
    g_state.musicTrackIndex = 0;
}

static bool pointInRect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}

void bottomscreen_handle_touch(touchPosition touch, bool touchHeld, bool touchDown) {
    bool justPressed = touchHeld && !s_wasTouching;
    s_wasTouching = touchHeld;
    if (!justPressed) { (void)touchDown; return; }

    if (g_state.showSettingsMenu) {
        // Bouton fermer (croix en haut à droite du panneau)
        if (pointInRect(touch.px, touch.py, GEAR_X, GEAR_Y, GEAR_SIZE, GEAR_SIZE)) {
            g_state.showSettingsMenu = false;
            return;
        }
        // Bouton musique on/off
        if (pointInRect(touch.px, touch.py, 30, 80, 140, 32)) {
            g_state.musicOn = !g_state.musicOn;
            return;
        }
        // Bouton piste suivante
        if (pointInRect(touch.px, touch.py, 180, 80, 110, 32)) {
            g_state.musicTrackIndex = (g_state.musicTrackIndex + 1) % 4;
            return;
        }
        // Sélecteur de couleur d'accent (5 pastilles)
        for (unsigned i = 0; i < ACCENT_COUNT; i++) {
            int cx = 36 + i * 56;
            int cy = 150;
            int dx = touch.px - cx, dy = touch.py - cy;
            if (dx*dx + dy*dy <= 20*20) {
                g_state.uiAccentColor = ACCENT_PRESETS[i];
                return;
            }
        }
    } else {
        if (pointInRect(touch.px, touch.py, GEAR_X, GEAR_Y, GEAR_SIZE, GEAR_SIZE)) {
            g_state.showSettingsMenu = true;
        }
    }
}

static void drawGearIcon(float x, float y, float size, u32 color) {
    float r = size * 0.5f;
    C2D_DrawCircleSolid(x + r, y + r, 0.0f, r * 0.55f, color);
    // dents simplifiées : 6 petits rectangles autour
    for (int i = 0; i < 6; i++) {
        float a = i * (M_PI / 3.0f);
        float tx = x + r + cosf(a) * r * 0.8f;
        float ty = y + r + sinf(a) * r * 0.8f;
        C2D_DrawRectSolid(tx - 3, ty - 3, 0.0f, 6, 6, color);
    }
}

static void drawLabel(C2D_TextBuf buf, float x, float y, float scale, u32 color, const char* str) {
    C2D_Text txt;
    C2D_TextParse(&txt, buf, str);
    C2D_TextOptimize(&txt);
    C2D_DrawText(&txt, C2D_WithColor, x, y, 0.0f, scale, scale, color);
}

static void renderMiniMap(void) {
    C2D_DrawRectSolid(0, 0, 0.0f, BOT_W, BOT_H, 0xFF0A0A12);

    float cx = BOT_W / 2.0f, cy = BOT_H / 2.0f;

    // Étoile centrale du système (toujours à peu près au centre des orbites, ici origine monde)
    C2D_DrawCircleSolid(cx - g_state.camX * 0.0f, cy, 0.0f, 5.0f, g_state.system.starColor);
    // NB: l'étoile est fixe à l'origine (0,0) du monde, donc sa position mini-map suit la caméra
    float starMapX = cx + (0.0f - g_state.ship.worldX) * MAP_SCALE;
    float starMapY = cy + (0.0f - g_state.ship.worldY) * MAP_SCALE;
    C2D_DrawCircleSolid(starMapX, starMapY, 0.0f, 5.0f, g_state.system.starColor);

    for (int i = 0; i < g_state.planetCount; i++) {
        Planet* p = &g_state.planets[i];
        float mx = cx + (p->x - g_state.ship.worldX) * MAP_SCALE;
        float my = cy + (p->y - g_state.ship.worldY) * MAP_SCALE;
        if (mx < -10 || mx > BOT_W + 10 || my < -10 || my > BOT_H + 10) continue;
        C2D_DrawCircleSolid(mx, my, 0.0f, 3.0f, p->baseColor);
        if (p->flagPlanted) {
            C2D_DrawCircleSolid(mx, my, 0.0f, 5.0f, 0x4000D9FF);
        }
    }

    // Vaisseau toujours au centre de la mini-map (la map "suit" le vaisseau)
    C2D_DrawTriangle(cx, cy - 5, 0xFFFFFFFF, cx - 4, cy + 4, 0xFFFFFFFF, cx + 4, cy + 4, 0xFFFFFFFF, 0.0f);

    drawGearIcon(GEAR_X, GEAR_Y, GEAR_SIZE, g_state.uiAccentColor);
}

static const char* TRACK_NAMES[] = { "Ambient 1", "Ambient 2", "Deep Space", "Silence" };

static void renderSettingsMenu(C2D_TextBuf buf) {
    C2D_DrawRectSolid(0, 0, 0.0f, BOT_W, BOT_H, 0xFF12121A);
    drawLabel(buf, 20, 16, 0.6f, g_state.uiAccentColor, "PARAMETRES");

    // bouton fermer
    C2D_DrawRectSolid(GEAR_X, GEAR_Y, 0.0f, GEAR_SIZE, GEAR_SIZE, 0xFF3A3A3A);
    drawLabel(buf, GEAR_X + 10, GEAR_Y + 6, 0.6f, 0xFFFFFFFF, "X");

    // Musique on/off
    drawLabel(buf, 30, 60, 0.45f, 0xFFAAAAAA, "MUSIQUE");
    C2D_DrawRectSolid(30, 80, 0.0f, 140, 32, g_state.musicOn ? g_state.uiAccentColor : 0xFF3A3A3A);
    drawLabel(buf, 70, 90, 0.5f, 0xFF101010, g_state.musicOn ? "ACTIVEE" : "COUPEE");

    // Piste suivante
    C2D_DrawRectSolid(180, 80, 0.0f, 110, 32, 0xFF3A3A3A);
    drawLabel(buf, 188, 90, 0.45f, 0xFFFFFFFF, "PISTE >");
    drawLabel(buf, 30, 118, 0.42f, 0xFF888888, TRACK_NAMES[g_state.musicTrackIndex]);

    // Couleur de l'interface
    drawLabel(buf, 30, 138, 0.45f, 0xFFAAAAAA, "COULEUR INTERFACE");
    for (unsigned i = 0; i < ACCENT_COUNT; i++) {
        int cx2 = 36 + i * 56, cy2 = 150;
        C2D_DrawCircleSolid(cx2, cy2, 0.0f, 18.0f, ACCENT_PRESETS[i]);
        if (ACCENT_PRESETS[i] == g_state.uiAccentColor) {
            C2D_DrawCircleSolid(cx2, cy2, 0.0f, 22.0f, 0x55FFFFFF);
            C2D_DrawCircleSolid(cx2, cy2, 0.0f, 18.0f, ACCENT_PRESETS[i]);
        }
    }

    drawLabel(buf, 30, 200, 0.4f, 0xFF666666, "Touchez l'ecran pour ajuster");
}

void bottomscreen_render(C2D_TextBuf textBuf) {
    C2D_TextBufClear(textBuf);
    if (g_state.showSettingsMenu) {
        renderSettingsMenu(textBuf);
    } else {
        renderMiniMap();
    }
}
