#pragma once
#include "game.h"

void bottomscreen_init(C2D_TextBuf* textBuf);
void bottomscreen_handle_touch(touchPosition touch, bool touchHeld, bool touchDown);
void bottomscreen_render(C2D_TextBuf textBuf);
