#pragma once
#include "game.h"

void ship_init(void);
void ship_update(float dt, circlePosition cpad, u32 kHeld);
void ship_render(void);
