#include <math.h>
#include "autopilot.h"

// ---------------------------------------------------------------
// autopilot.c
// Quand AUTO est activé, le jeu choisit une planète non visitée
// (sans drapeau planté) la plus proche et simule une entrée de
// Circle Pad en direction de celle-ci. Une fois toutes les
// planètes visitées, il flotte doucement sans but (le système est
// "exploré").
// ---------------------------------------------------------------

static int pickTarget(void) {
    Ship* sh = &g_state.ship;
    int best = -1;
    float bestDist = 1e9f;
    for (int i = 0; i < g_state.planetCount; i++) {
        Planet* p = &g_state.planets[i];
        if (p->flagPlanted) continue;
        float dx = p->x - sh->worldX, dy = p->y - sh->worldY;
        float d = vecLength(dx, dy);
        if (d < bestDist) { bestDist = d; best = i; }
    }
    return best;
}

circlePosition autopilot_compute_input(void) {
    circlePosition fake = { 0, 0 };
    if (!g_state.autoPilotOn) return fake;

    g_state.autoPilotTargetPlanet = pickTarget();
    if (g_state.autoPilotTargetPlanet < 0) return fake; // tout est visité

    Planet* target = &g_state.planets[g_state.autoPilotTargetPlanet];
    Ship* sh = &g_state.ship;

    float dx = target->x - sh->worldX;
    float dy = target->y - sh->worldY;
    float dist = vecLength(dx, dy);
    if (dist < 1.0f) return fake;

    dx /= dist; dy /= dist;

    // Ralentit en approche pour laisser l'atterrissage automatique se déclencher
    float intensity = (dist < target->radius + 60.0f) ? 0.35f : 1.0f;

    fake.dx = (s16)(dx * 150.0f * intensity);
    fake.dy = (s16)(dy * 150.0f * intensity);
    return fake;
}
