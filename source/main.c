#include <3ds.h>
#include <citro2d.h>
#include <citro3d.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "starfield.h"
#include "ship.h"
#include "planet.h"
#include "hud.h"
#include "bottomscreen.h"
#include "autopilot.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    C3D_RenderTarget* topScreen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
    C3D_RenderTarget* botScreen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

    srand((unsigned)svcGetSystemTick());

    g_state.frameCount = 0;
    g_state.camX = 0.0f;
    g_state.camY = 0.0f;
    g_state.autoPilotOn = false;
    g_state.autoPilotTargetPlanet = -1;
    g_state.showSettingsMenu = false;

    ship_init();
    starfield_init();
    planets_generate_system();

    C2D_TextBuf hudTextBuf;
    hud_init(&hudTextBuf);
    C2D_TextBuf botTextBuf;
    bottomscreen_init(&botTextBuf);

    bool prevY = false;

    while (aptMainLoop()) {
        hidScanInput();
        u32 kHeld = hidKeysHeld();
        u32 kDown = hidKeysDown();
        circlePosition cpad;
        hidCircleRead(&cpad);

        if (kDown & KEY_START) break;

        bool yHeld = (kHeld & KEY_Y) != 0;
        if (yHeld && !prevY) {
            g_state.autoPilotOn = !g_state.autoPilotOn;
        }
        prevY = yHeld;

        touchPosition touch;
        hidTouchRead(&touch);
        bool touchHeld = (kHeld & KEY_TOUCH) != 0;
        bool touchDown = (kDown & KEY_TOUCH) != 0;
        bottomscreen_handle_touch(touch, touchHeld, touchDown);

        float dt = 1.0f / 60.0f;

        circlePosition effectiveCpad = cpad;
        if (g_state.autoPilotOn) {
            effectiveCpad = autopilot_compute_input();
        }

        ship_update(dt, effectiveCpad, g_state.autoPilotOn ? 0 : kHeld);

        g_state.camX = g_state.ship.worldX;
        g_state.camY = g_state.ship.worldY;

        float speed = vecLength(g_state.ship.vx, g_state.ship.vy);
        starfield_update(dt, speed);

        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

        C2D_TargetClear(topScreen, 0xFF000000);
        C2D_SceneBegin(topScreen);

        starfield_render(hudTextBuf);
        planets_render();
        ship_render();
        hud_render(hudTextBuf);

        C2D_TargetClear(botScreen, 0xFF000000);
        C2D_SceneBegin(botScreen);
        bottomscreen_render(botTextBuf);

        C3D_FrameEnd(0);
        g_state.frameCount++;
    }

    C2D_TextBufDelete(hudTextBuf);
    C2D_TextBufDelete(botTextBuf);
    C2D_Fini();
    C3D_Fini();
    gfxExit();
    return 0;
}
