#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "hud.h"
#include "planet.h"

// ---------------------------------------------------------------
// hud.c
// Panneau d'informations façon "interface scientifique" affiché
// en overlay en bas de l'écran du HAUT (par-dessus le rendu du
// jeu) : vitesse, puissance propulseurs, coordonnées, infos sur
// l'étoile du système, et le bouton AUTO (pilote automatique).
// Le bouton AUTO est cliquable... mais l'écran du haut n'est pas
// tactile sur 3DS, donc on l'active avec la touche Y (cf. main.c)
// et son état visuel est juste affiché ici.
// ---------------------------------------------------------------

#define HUD_HEIGHT 56
#define HUD_Y      (TOP_H - HUD_HEIGHT)

void hud_init(C2D_TextBuf* textBuf) {
    *textBuf = C2D_TextBufNew(2048);
}

static void drawLabel(C2D_TextBuf buf, float x, float y, float scale, u32 color, const char* fmt, ...) {
    char str[64];
    va_list args;
    va_start(args, fmt);
    vsnprintf(str, sizeof(str), fmt, args);
    va_end(args);

    C2D_Text txt;
    C2D_TextParse(&txt, buf, str);
    C2D_TextOptimize(&txt);
    C2D_DrawText(&txt, C2D_WithColor, x, y, 0.0f, scale, scale, color);
}

void hud_render(C2D_TextBuf textBuf) {
    C2D_TextBufClear(textBuf);

    Ship* sh = &g_state.ship;
    float speed = vecLength(sh->vx, sh->vy);

    // Panneau de fond semi-transparent
    C2D_DrawRectSolid(0, HUD_Y, 0.0f, TOP_W, HUD_HEIGHT, 0xCC1A1A1A);
    C2D_DrawLine(0, HUD_Y, g_state.uiAccentColor, TOP_W, HUD_Y, g_state.uiAccentColor, 1.5f, 0.0f);

    u32 textColor = 0xFFEFEFEF;
    u32 accentColor = g_state.uiAccentColor;

    // Colonne 1 : vitesse + puissance propulseurs
    drawLabel(textBuf, 6, HUD_Y + 4, 0.45f, accentColor, "VITESSE");
    drawLabel(textBuf, 6, HUD_Y + 16, 0.5f, textColor, "%.0f u/s", speed);
    drawLabel(textBuf, 6, HUD_Y + 32, 0.45f, accentColor, "PROPULSEURS");
    drawLabel(textBuf, 6, HUD_Y + 44, 0.5f, textColor, "%3.0f%%", sh->thrustPower * 100.0f);

    // Colonne 2 : coordonnées
    drawLabel(textBuf, 110, HUD_Y + 4, 0.45f, accentColor, "COORDONNEES");
    drawLabel(textBuf, 110, HUD_Y + 16, 0.45f, textColor, "X %.0f", sh->worldX);
    drawLabel(textBuf, 110, HUD_Y + 28, 0.45f, textColor, "Y %.0f", sh->worldY);
    drawLabel(textBuf, 110, HUD_Y + 40, 0.45f, sh->isLanded ? 0xFF6FE0FF : 0xFF888888,
              sh->isLanded ? "POSE" : "EN VOL");

    // Colonne 3 : infos étoile du système
    drawLabel(textBuf, 210, HUD_Y + 4, 0.45f, accentColor, "%s", g_state.system.starName);
    drawLabel(textBuf, 210, HUD_Y + 16, 0.4f, textColor, "%s", g_state.system.starType);
    drawLabel(textBuf, 210, HUD_Y + 28, 0.4f, textColor, "Rayon %.1f Rs", g_state.system.starRadius);
    drawLabel(textBuf, 210, HUD_Y + 40, 0.4f, textColor, "Masse %.1f Ms  Corps:%d",
              g_state.system.starMass, g_state.system.bodyCount);

    // Bouton AUTO (visuel) - activé via touche Y
    float btnX = TOP_W - 60, btnY = HUD_Y + 10, btnW = 52, btnH = 36;
    u32 btnColor = g_state.autoPilotOn ? accentColor : 0xFF3A3A3A;
    C2D_DrawRectSolid(btnX, btnY, 0.0f, btnW, btnH, btnColor);
    drawLabel(textBuf, btnX + 8, btnY + 10, 0.55f, g_state.autoPilotOn ? 0xFF101010 : 0xFFAAAAAA, "AUTO");

    // Si une planète est ciblée par l'auto-pilote ou survolée de près, afficher ses stats
    int nearest = -1;
    float nearestDist = 1e9f;
    for (int i = 0; i < g_state.planetCount; i++) {
        Planet* p = &g_state.planets[i];
        float dx = p->x - sh->worldX, dy = p->y - sh->worldY;
        float d = vecLength(dx, dy);
        if (d < nearestDist) { nearestDist = d; nearest = i; }
    }
    if (nearest >= 0 && nearestDist < 200.0f) {
        Planet* p = &g_state.planets[nearest];
        drawLabel(textBuf, 6, HUD_Y - 14, 0.42f, accentColor,
                  "%s | %s | R:%.1f | M:%.1fT | Corps:%.0f",
                  p->name, planet_type_name(p->type), p->radius / 10.0f, p->massEarth, p->orbitingBodies);
    }
}
