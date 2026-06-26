#pragma once
#include <citro2d.h>
#include <citro3d.h>
#include <3ds.h>

// ---------------------------------------------------------------
// game.h — structures de données globales partagées par tous les
// modules du jeu (vaisseau, planètes, étoiles, HUD, écran du bas)
// ---------------------------------------------------------------

#define MAX_STARS        400
#define MAX_PLANETS      9
#define MAX_TRAIL_POINTS 12

#define TOP_W 400
#define TOP_H 240
#define BOT_W 320
#define BOT_H 240

// --- Types de planètes (détermine apparence ET stats simulées) ---
typedef enum {
    PLANET_ROCKY = 0,
    PLANET_DESERT,
    PLANET_OCEAN,
    PLANET_GAS_GIANT,
    PLANET_TYPE_COUNT
} PlanetType;

typedef struct {
    float x, y;          // position dans le système (coordonnées monde)
    float radius;        // rayon visuel/physique
    PlanetType type;
    C2D_Image image;      // sprite généré (sur HUD: 256x256 max ramené au radius)
    u32 baseColor;        // couleur de base (teinte dominante)
    u32 secondColor;       // couleur secondaire (bandes, cratères, etc.)
    bool hasRings;
    float ringAngle;       // inclinaison visuelle des anneaux
    float massEarth;       // masse simulée (en masses terrestres)
    float orbitingBodies;  // nb de lunes/corps en orbite (simulé)
    char name[16];
    bool flagPlanted;      // drapeau déjà planté ici ?
} Planet;

typedef struct {
    float x, y;            // position écran (relatif à la caméra)
    float worldX, worldY;  // position monde
    float brightness;      // 0..1, pour le scintillement
    float twinkleSpeed;
    float size;
    u32 color;
    bool twinkles;          // certaines étoiles scintillent, d'autres non
} Star;

typedef struct {
    float worldX, worldY;   // position dans le monde
    float vx, vy;            // vélocité
    float angle;              // orientation en radians (0 = vers le haut)
    float targetAngle;
    float thrustPower;       // 0..1 puissance actuelle des propulseurs
    bool isThrusting;
    bool isLanded;
    int  landedPlanetIdx;     // -1 si pas posé
    float trailX[MAX_TRAIL_POINTS];
    float trailY[MAX_TRAIL_POINTS];
    int  trailCount;
} Ship;

typedef struct {
    char starName[16];
    char starType[12];        // "Naine jaune", "Géante bleue", etc.
    float starRadius;          // rayon de l'étoile centrale (en rayons solaires)
    float starMass;            // masse de l'étoile (en masses solaires)
    u32 starColor;
    int  bodyCount;             // nb total de planètes dans le système
} StarSystemInfo;

// État global partagé entre les modules (un seul système actif à la fois,
// suffisant pour la 3DS et pour le scope demandé)
typedef struct {
    Ship ship;
    Star stars[MAX_STARS];
    Planet planets[MAX_PLANETS];
    int planetCount;
    StarSystemInfo system;

    float camX, camY;     // caméra suit le vaisseau
    bool autoPilotOn;
    int  autoPilotTargetPlanet;

    // réglages (modifiables depuis l'écran du bas)
    u32 uiAccentColor;     // couleur de l'interface (paramétrable)
    bool musicOn;
    int  musicTrackIndex;

    bool showSettingsMenu; // overlay réglages actif sur l'écran du bas
    u64  frameCount;
} GameState;

extern GameState g_state;

// Utilitaires communs
float  randf(float min, float max);
u32    lerpColor(u32 c1, u32 c2, float t);
float  vecLength(float x, float y);
void   vecNormalize(float* x, float* y);
