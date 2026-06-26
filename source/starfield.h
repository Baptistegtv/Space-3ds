#pragma once
#include "game.h"

void starfield_init(void);
void starfield_update(float dt, float shipSpeed);
void starfield_render(C2D_TextBuf textBuf);
