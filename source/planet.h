#pragma once
#include "game.h"

void planets_generate_system(void); // génère un système complet (étoile + planètes)
void planets_render(void);
const char* planet_type_name(PlanetType t);
