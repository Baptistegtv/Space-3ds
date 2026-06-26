#include <math.h>
#include "ship.h"

// ---------------------------------------------------------------
// ship.c
// Vaisseau vu de dessus, contrôlable au Circle Pad ET au D-Pad
// (les deux sont lus et sommés, donc le joueur utilise celui qu'il
// préfère, ou même les deux en même temps).
// Rotation fluide vers l'angle visé (lerp d'angle), flamme du
// moteur visible quand on accélère, atterrissage automatique +
// drapeau plantés quand on est assez proche/lentement d'une planète.
// ---------------------------------------------------------------

#define SHIP_ACCEL          220.0f   // accélération px/s^2 à pleine poussée
#define SHIP_MAX_SPEED       260.0f
#define SHIP_FRICTION         0.992f  // léger freinage naturel (style "spatial" mais pas du tout du 100% inertie pour rester confortable au pad)
#define SHIP_ROT_SPEED         6.0f    // vitesse de rotation (rad/s) vers l'angle cible
#define LANDING_DISTANCE       4.0f    // distance (en multiples du rayon planète) pour déclencher l'atterrissage
#define LANDING_MAX_SPEED      40.0f

void ship_init(void) {
    Ship* sh = &g_state.ship;
    sh->worldX = 0.0f;
    sh->worldY = 0.0f;
    sh->vx = 0.0f;
    sh->vy = 0.0f;
    sh->angle = 0.0f;
    sh->targetAngle = 0.0f;
    sh->thrustPower = 0.0f;
    sh->isThrusting = false;
    sh->isLanded = false;
    sh->landedPlanetIdx = -1;
    sh->trailCount = 0;
}

static float angleDiff(float a, float b) {
    float d = fmodf(b - a + M_PI, 2.0f * M_PI);
    if (d < 0) d += 2.0f * M_PI;
    return d - M_PI;
}

static void checkAutoLanding(void) {
    Ship* sh = &g_state.ship;
    float speed = vecLength(sh->vx, sh->vy);
    if (speed > LANDING_MAX_SPEED) return;

    for (int i = 0; i < g_state.planetCount; i++) {
        Planet* p = &g_state.planets[i];
        float dx = p->x - sh->worldX;
        float dy = p->y - sh->worldY;
        float dist = vecLength(dx, dy);
        if (dist < p->radius + LANDING_DISTANCE) {
            sh->isLanded = true;
            sh->landedPlanetIdx = i;
            sh->vx = 0; sh->vy = 0;
            p->flagPlanted = true;
            return;
        }
    }
}

void ship_update(float dt, circlePosition cpad, u32 kHeld) {
    Ship* sh = &g_state.ship;

    // --- Entrée combinée : Circle Pad (analogique) + D-Pad (digital) ---
    float inputX = 0.0f, inputY = 0.0f;

    // Circle Pad : valeurs entre -156 et +156 environ sur 3DS
    if (cpad.dx > 10 || cpad.dx < -10) inputX += cpad.dx / 156.0f;
    if (cpad.dy > 10 || cpad.dy < -10) inputY += cpad.dy / 156.0f;

    if (kHeld & KEY_DLEFT)  inputX -= 1.0f;
    if (kHeld & KEY_DRIGHT) inputX += 1.0f;
    if (kHeld & KEY_DUP)    inputY += 1.0f;
    if (kHeld & KEY_DDOWN)  inputY -= 1.0f;

    // Clamp pour éviter une diagonale plus rapide qu'un déplacement droit
    float inputLen = vecLength(inputX, inputY);
    if (inputLen > 1.0f) { inputX /= inputLen; inputY /= inputLen; }

    bool hasInput = inputLen > 0.05f;

    // Redécollage si on bouge alors qu'on est posé
    if (sh->isLanded && hasInput) {
        sh->isLanded = false;
        sh->landedPlanetIdx = -1;
    }

    if (!sh->isLanded) {
        if (hasInput) {
            sh->targetAngle = atan2f(inputX, inputY); // 0 = vers le haut de l'écran
            sh->thrustPower = inputLen;
            sh->isThrusting = true;

            float diff = angleDiff(sh->angle, sh->targetAngle);
            sh->angle += diff * fminf(SHIP_ROT_SPEED * dt, 1.0f);

            // On accélère dans la direction où le vaisseau POINTE, pas l'input brut,
            // pour donner une vraie sensation de pilotage façon vaisseau spatial.
            float thrustX = sinf(sh->angle);
            float thrustY = cosf(sh->angle);
            sh->vx += thrustX * SHIP_ACCEL * inputLen * dt;
            sh->vy += thrustY * SHIP_ACCEL * inputLen * dt;
        } else {
            sh->isThrusting = false;
            sh->thrustPower *= 0.9f;
        }

        // Friction légère + limite de vitesse
        sh->vx *= SHIP_FRICTION;
        sh->vy *= SHIP_FRICTION;
        float speed = vecLength(sh->vx, sh->vy);
        if (speed > SHIP_MAX_SPEED) {
            sh->vx = sh->vx / speed * SHIP_MAX_SPEED;
            sh->vy = sh->vy / speed * SHIP_MAX_SPEED;
        }

        sh->worldX += sh->vx * dt;
        sh->worldY += sh->vy * dt;

        checkAutoLanding();
    }

    // --- Traînée (petite ligne de positions passées pour un sillage léger) ---
    if (g_state.frameCount % 2 == 0) {
        for (int i = MAX_TRAIL_POINTS - 1; i > 0; i--) {
            sh->trailX[i] = sh->trailX[i - 1];
            sh->trailY[i] = sh->trailY[i - 1];
        }
        sh->trailX[0] = sh->worldX;
        sh->trailY[0] = sh->worldY;
        if (sh->trailCount < MAX_TRAIL_POINTS) sh->trailCount++;
    }
}

void ship_render(void) {
    Ship* sh = &g_state.ship;
    float sx = sh->worldX - g_state.camX + TOP_W / 2.0f;
    float sy = sh->worldY - g_state.camY + TOP_H / 2.0f;

    // Sillage discret derrière le vaisseau
    for (int i = 0; i < sh->trailCount - 1; i++) {
        float ax = sh->trailX[i] - g_state.camX + TOP_W / 2.0f;
        float ay = sh->trailY[i] - g_state.camY + TOP_H / 2.0f;
        float bx = sh->trailX[i + 1] - g_state.camX + TOP_W / 2.0f;
        float by = sh->trailY[i + 1] - g_state.camY + TOP_H / 2.0f;
        float alpha = (1.0f - (float)i / MAX_TRAIL_POINTS) * 0.25f;
        u32 col = (u32)(alpha * 255) << 24 | 0x00CCE5FF;
        C2D_DrawLine(ax, ay, col, bx, by, col, 1.5f, 0.0f);
    }

    // Flamme du moteur (dessinée avant le corps, derrière, orientée à l'opposé du nez)
    if (sh->isThrusting && sh->thrustPower > 0.05f) {
        float flameLen = 6.0f + sh->thrustPower * 10.0f;
        float backX = sx - sinf(sh->angle) * 7.0f;
        float backY = sy - cosf(sh->angle) * 7.0f;
        float tipX  = backX - sinf(sh->angle) * flameLen;
        float tipY  = backY - cosf(sh->angle) * flameLen;
        u32 flameColorOuter = 0xFF3D7CFF; // orange-jaune (ABGR: 0xAABBGGRR)
        u32 flameColorInner = 0xFF55D9FF;
        C2D_DrawLine(backX, backY, flameColorOuter, tipX, tipY, flameColorInner, 3.5f * sh->thrustPower, 0.0f);
    }

    // Corps du vaisseau : petit triangle blanc épuré (vu de dessus)
    float cosA = cosf(sh->angle), sinA = sinf(sh->angle);
    float noseX  = sx + sinA * 9.0f,  noseY  = sy + cosA * 9.0f;
    float leftX  = sx - cosA * 6.0f - sinA * -6.0f, leftY = sy + sinA * 6.0f - cosA * -6.0f;
    float rightX = sx + cosA * 6.0f - sinA * -6.0f, rightY = sy - sinA * 6.0f - cosA * -6.0f;

    C2D_DrawTriangle(noseX, noseY, 0xFFFFFFFF,
                      leftX, leftY, 0xFFDDDDDD,
                      rightX, rightY, 0xFFDDDDDD, 0.0f);

    // Drapeau si posé sur une planète
    if (sh->isLanded && sh->landedPlanetIdx >= 0) {
        C2D_DrawLine(sx, sy, 0xFFAAAAAA, sx, sy - 10, 0xFFAAAAAA, 1.0f, 0.0f);
        C2D_DrawTriangle(sx, sy - 10, 0xFF00D9FF,
                          sx + 6, sy - 8, 0xFF00D9FF,
                          sx, sy - 6, 0xFF00D9FF, 0.0f);
    }
}
