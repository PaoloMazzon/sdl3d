#include "Structs.h"
#pragma once

void levelCreate(GameState *game);
void levelDestroy(GameState *game);
bool levelUpdate(GameState *game);
void levelDraw(GameState *game);
void levelDrawUI(GameState *game);
bool touchingWall(GameState *game, Level *level, trs_Hitbox hitbox, float x, float y, float z);
void addWall(Level *level, Wall *wall);