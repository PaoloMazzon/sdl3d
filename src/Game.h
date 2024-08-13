#include "Structs.h"
#pragma once

void gameSave(GameState *game);
void gameStart(GameState *game);
void gameEnd(GameState *game);
bool gameUpdate(GameState *game);
void gameDraw(GameState *game);
void gameUI(GameState *game);