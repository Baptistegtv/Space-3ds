#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "planet.h"

// ---------------------------------------------------------------
// planet.c
// Génération procédurale légère : chaque planète reçoit un type,
// une couleur de base + secondaire, un rayon, une masse simulée
// et un nombre de corps en orbite. Le rendu se fait avec des
// primitives C2D (cercles, arcs simulés via segments) plutôt que
// des textures chargées, ce qui évite d'avoir à gérer des sprites
// et reste très léger pour la 3DS.
// ---------------------------------------------------------------

static const char* STAR_NAMES[] = {
    "Kepler-7", "Vega-Theta", "Aldra", "Nyx-12", "Solenne",
    "Ourano-9", "Vesperia", "Thelios", "Cassia-4", "Ombra"
};

static const char* PLANET_TYPE_NAMES[] = {
    "Rocheuse", "Desertique", "Oceanique", "Geante gazeuse"
};

const char* planet_type_name(PlanetType t) {
    return PLANET_TYPE_NAMES[t];
}

// Palette de couleurs par type (format ABGR attendu par citro2d : 0xAABBGGRR)
static void colorsForType(PlanetType type, u32* base, u32* secondary) {
    switch (type) {
        case PLANET_ROCKY:
            *base = 0xFF8A8A93;       // gris
            *secondary = 0xFF5E5E66;  // gris foncé (cratères)
            break;
        case PLANET_DESERT:
            *base = 0xFF3D6FCB;       // orange-rouge en ABGR -> attention ordre
            *secondary = 0xFF2A4E9E;
            break;
        case PLANET_OCEAN:
            *base = 0xFFD98A3C;       // bleu océan en ABGR
            *secondary = 0xFFB8651F;
            break;
        case PLANET_GAS_GIANT:
            *base = 0xFF6FC9E0;       // teinte chaude/bandes en ABGR
            *secondary = 0xFF4FA3D9;
            break;
        default:
            *base = 0xFFFFFFFF; *secondary = 0xFFCCCCCC;
            break;
    }
}

static StarSystemInfo generateStarInfo(void) {
    StarSystemInfo info;
    int nameIdx = rand() % (sizeof(STAR_NAMES) / sizeof(char*));
    strncpy(info.starName, STAR_NAMES[nameIdx], sizeof(info.starName) - 1);
    info.starName[sizeof(info.starName)-1] = '\0';

    float r = randf(0.0f, 1.0f);
    if (r < 0.5f) { strcpy(info.starType, "Naine jaune"); info.starColor = 0xFF6FE0FF; }
    else if (r < 0.75f) { strcpy(info.starType, "Naine rouge"); info.starColor = 0xFF4F6BFF; }
    else if (r < 0.9f) { strcpy(info.starType, "Geante bleue"); info.starColor = 0xFFFFB85C; }
    else { strcpy(info.starType, "Geante orange"); info.starColor = 0xFF3D9CFF; }

    info.starRadius = randf(0.6f, 3.2f);
    info.starMass = randf(0.4f, 8.0f);
    info.bodyCount = 0; // rempli après génération des planètes
    return info;
}

void planets_generate_system(void) {
    g_state.system = generateStarInfo();
    g_state.planetCount = 3 + rand() % (MAX_PLANETS - 2); // entre 3 et MAX_PLANETS

    float orbitRadius = 220.0f;
    for (int i = 0; i < g_state.planetCount; i++) {
        Planet* p = &g_state.planets[i];
        p->type = (PlanetType)(rand() % PLANET_TYPE_COUNT);
        colorsForType(p->type, &p->baseColor, &p->secondColor);

        p->radius = (p->type == PLANET_GAS_GIANT) ? randf(28.0f, 42.0f) : randf(12.0f, 24.0f);

        float angle = randf(0.0f, 2.0f * M_PI);
        orbitRadius += p->radius * 2.0f + randf(60.0f, 140.0f);
        p->x = cosf(angle) * orbitRadius;
        p->y = sinf(angle) * orbitRadius;

        p->hasRings = (p->type == PLANET_GAS_GIANT && (rand() % 100) < 55) ||
                      (rand() % 100) < 12;
        p->ringAngle = randf(0.15f, 0.45f);

        p->massEarth = (p->type == PLANET_GAS_GIANT) ? randf(50.0f, 300.0f) : randf(0.1f, 8.0f);
        p->orbitingBodies = (float)(rand() % 6);
        p->flagPlanted = false;

        snprintf(p->name, sizeof(p->name), "%s-%d", g_state.system.starName, i + 1);
    }
    g_state.system.bodyCount = g_state.planetCount;
}

static inline u32 colorWithAlpha(u32 color, float alphaMul) {
    u8 a = (u8)(((color >> 24) & 0xFF) * alphaMul);
    return (color & 0x00FFFFFF) | (a << 24);
}

// Dessine un disque avec un léger dégradé radial simulé par 2-3 cercles concentriques
static void drawPlanetBody(float sx, float sy, float r, u32 base, u32 secondary, PlanetType type, int seed) {
    C2D_DrawCircleSolid(sx, sy, 0.0f, r, base);

    switch (type) {
        case PLANET_ROCKY: {
            // quelques "cratères" : petits disques plus sombres, positions pseudo-fixes (seed)
            int craterCount = 3 + (seed % 3);
            for (int i = 0; i < craterCount; i++) {
                float a = (i * 2.39996f) + seed; // angle pseudo-aléatoire stable
                float dist = r * (0.25f + 0.5f * ((i * 37 + seed) % 100) / 100.0f);
                float cx = sx + cosf(a) * dist;
                float cy = sy + sinf(a) * dist;
                float cr = r * 0.12f * (0.6f + 0.4f * ((i * 53 + seed) % 100) / 100.0f);
                C2D_DrawCircleSolid(cx, cy, 0.0f, cr, secondary);
            }
            break;
        }
        case PLANET_DESERT: {
            // bandes de poussière horizontales légères
            for (int i = -1; i <= 1; i++) {
                C2D_DrawEllipseSolid(sx, sy + i * r * 0.45f, 0.0f, r * 1.5f, r * 0.22f, colorWithAlpha(secondary, 0.55f));
            }
            break;
        }
        case PLANET_OCEAN: {
            // "continents" : taches irrégulières via petits cercles superposés
            int spots = 3 + (seed % 2);
            for (int i = 0; i < spots; i++) {
                float a = (i * 2.1f) + seed * 0.5f;
                float dist = r * 0.4f;
                float cx = sx + cosf(a) * dist;
                float cy = sy + sinf(a) * dist;
                C2D_DrawCircleSolid(cx, cy, 0.0f, r * 0.28f, colorWithAlpha(secondary, 0.7f));
            }
            break;
        }
        case PLANET_GAS_GIANT: {
            // bandes horizontales colorées, façon Jupiter/Saturne simplifiée
            int bandCount = 4;
            for (int i = 0; i < bandCount; i++) {
                float t = -1.0f + (2.0f * i) / (bandCount - 1);
                float bandY = sy + t * r * 0.65f;
                float bandH = r * 0.22f;
                u32 bandColor = (i % 2 == 0) ? secondary : colorWithAlpha(base, 0.85f);
                C2D_DrawEllipseSolid(sx, bandY, 0.0f, r * 0.98f, bandH, bandColor);
            }
            break;
        }
        default: break;
    }

    // Léger dégradé/ombrage : un voile sombre sur le bord inférieur-droit pour donner du volume
    C2D_DrawCircleSolid(sx + r * 0.18f, sy + r * 0.18f, 0.0f, r * 0.92f, colorWithAlpha(0xFF000000, 0.10f));
}

static void drawRings(float sx, float sy, float r, float tilt, int seed) {
    u32 ringColor = colorWithAlpha(0xFFD9D9D9, 0.55f);
    u32 ringColor2 = colorWithAlpha(0xFFBFBFBF, 0.35f);
    float outerRX = r * 1.9f, outerRY = r * 1.9f * tilt;
    float innerRX = r * 1.5f, innerRY = r * 1.5f * tilt;
    (void)seed;
    // Anneau dessiné comme deux ellipses fines concentriques (creux simulé par superposition de couleur de fond derrière la planète)
    C2D_DrawEllipseSolid(sx, sy, 0.0f, outerRX, outerRY * 0.18f, ringColor);
    C2D_DrawEllipseSolid(sx, sy, 0.0f, innerRX, innerRY * 0.16f, ringColor2);
}

void planets_render(void) {
    for (int i = 0; i < g_state.planetCount; i++) {
        Planet* p = &g_state.planets[i];
        float sx = p->x - g_state.camX + TOP_W / 2.0f;
        float sy = p->y - g_state.camY + TOP_H / 2.0f;

        if (sx < -p->radius * 3 || sx > TOP_W + p->radius * 3 ||
            sy < -p->radius * 3 || sy > TOP_H + p->radius * 3) continue;

        // Anneaux derrière si l'angle le justifie (simplifié : toujours derrière, fin sur l'avant via 2e passe)
        if (p->hasRings) drawRings(sx, sy, p->radius, p->ringAngle, i);

        drawPlanetBody(sx, sy, p->radius, p->baseColor, p->secondColor, p->type, i * 7 + 3);

        if (p->hasRings) {
            // fine bande d'anneau qui repasse devant la moitié basse pour un effet "autour" de la planète
            u32 ringFront = colorWithAlpha(0xFFE6E6E6, 0.4f);
            C2D_DrawEllipseSolid(sx, sy + p->radius * 1.5f * p->ringAngle * 0.05f, 0.0f,
                                  p->radius * 1.55f, p->radius * 1.55f * p->ringAngle * 0.16f, ringFront);
        }

        // Petit drapeau jaune permanent si déjà visité
        if (p->flagPlanted) {
            C2D_DrawLine(sx, sy - p->radius, 0xFFAAAAAA, sx, sy - p->radius - 8, 0xFFAAAAAA, 1.0f, 0.0f);
            C2D_DrawTriangle(sx, sy - p->radius - 8, 0xFF00D9FF,
                              sx + 5, sy - p->radius - 6, 0xFF00D9FF,
                              sx, sy - p->radius - 4, 0xFF00D9FF, 0.0f);
        }
    }
}
