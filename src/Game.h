#include "Structs.h"
#pragma once

void gameSave(GameState *game);
void gameStart(GameState *game);
void gameEnd(GameState *game);
bool gameUpdate(GameState *game);
void gameDraw(GameState *game);
void gameUI(GameState *game);
SaveLevelInfo *gameSaveGetScores(GameState *game, const char *levelName);
// Returns true if time is a highscore
bool gameSaveSetScores(GameState *game, const char *levelName, float *checkpointTimes, int checkpointCount, float time);