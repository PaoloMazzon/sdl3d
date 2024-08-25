#include <SDL2/SDL.h>
#include "cJSON.h"
#include "Software3D.h"
#include "Game.h"
#include "Level.h"
#include "Player.h"
#include "Menu.h"

const char const *SAVE_FILE = "game.sav";
const uint32_t SAVE_VERSION = 1;

void gameSave(GameState *game) {
    // TODO: This
}

void gameLoad(GameState *game) {
    int size;
    uint8_t *file = trs_LoadFile("game.sav", &size);
    if (file != NULL) {
        file = realloc(file, size + 1);
        file[size] = 0;
        cJSON *json = cJSON_Parse(file);

        // TODO: This
    }
}

void gameStart(GameState *game) {
    // Load save
    gameLoad(game);

    // Basic game assets
    game->font = trs_LoadFont("res/font.png", 7, 8);
    game->menuFont = trs_LoadFont("res/font2.png", 16, 16);
    game->compassTex = trs_LoadPNG("res/compass.png");
    game->playerModel = trs_LoadModel("res/player.obj");
    game->platformModel = trs_LoadModel("res/platform.obj");
    game->islandModel = trs_LoadModel("res/island.obj");
    game->hintTex = trs_LoadPNG("res/hint.png");
    game->flagModel = trs_LoadModel("res/flag.obj");

    // Ground plane
    const float groundSize = 2;
    trs_Vertex vertices[] = {
        {{-groundSize, -groundSize, 0, 1},      { 88 / 128.0f,  0 / 128.0f}},
        {{groundSize, -groundSize, 0, 1},       {104 / 128.0f,  0 / 128.0f}},
        {{-groundSize, groundSize, 0, 1},       { 88 / 128.0f, 16 / 128.0f}},
        {{groundSize, -groundSize, 0, 1},       {104 / 128.0f,  0 / 128.0f}},
        {{groundSize, groundSize, 0, 1},        {104 / 128.0f, 16 / 128.0f}},
        {{-groundSize, groundSize, 0, 1},       { 88 / 128.0f, 16 / 128.0f}},
    };
    game->groundPlane = trs_CreateModel(vertices, 6);

    // Test model
    const float size = 1;
    trs_Vertex vl[] = {
        {{-size, -size, 0, 1},     { 8 / 128.0f,  0 / 128.0f}},
        {{size, -size, 0, 1},      {24 / 128.0f,  0 / 128.0f}},
        {{-size, size, 0, 1},      { 8 / 128.0f, 16 / 128.0f}},
        {{size, -size, 0, 1},      {24 / 128.0f,  0 / 128.0f}},
        {{size, size, 0, 1},       {24 / 128.0f, 16 / 128.0f}},
        {{-size, size, 0, 1},      { 8 / 128.0f, 16 / 128.0f}},
        {{-size, -size, 0, 1},     {24 / 128.0f,  0 / 128.0f}},
        {{-size, -size, -size, 1}, {24 / 128.0f,  8 / 128.0f}},
        {{-size, size, 0, 1},      { 8 / 128.0f,  0 / 128.0f}},
        {{-size, size, 0, 1},      { 8 / 128.0f,  0 / 128.0f}},
        {{-size, size, -size, 1},  { 8 / 128.0f,  8 / 128.0f}},
        {{-size, -size, -size, 1}, {24 / 128.0f,  8 / 128.0f}},
        {{-size + 5, -size, 0, 1}, {40 / 128.0f,  0 / 128.0f}},
        {{size + 5, -size, 0, 1},  {56 / 128.0f,  0 / 128.0f}},
        {{-size + 5, size, 0, 1},  {40 / 128.0f, 16 / 128.0f}},
        {{-size + 5, size, -1, 1}, {56 / 128.0f,  0 / 128.0f}},
        {{-size + 5, size, 0, 1},  {56 / 128.0f,  8 / 128.0f}},
        {{size + 5, size, 0, 1},   {72 / 128.0f,  8 / 128.0f}}
    };
    game->testModel = trs_CreateModel(vl, 18);
    
    // Player
    menuStart(game, &game->menu);
    playerCreate(game, &game->player);
}

void gameEnd(GameState *game) {
    gameSave(game);
    // Free the saves
    for (int i = 0; i < game->save.saveCount; i++) {
        free(game->save.saves[i].checkpointBestTimes);
        free((void*)game->save.saves[i].levelName);
    }

    levelDestroy(game);
    playerDestroy(game, &game->player);
    trs_FreeFont(game->font);
    trs_FreeFont(game->menuFont);
    trs_FreeModel(game->testModel);
    trs_FreeModel(game->groundPlane);
    trs_FreeModel(game->platformModel);
    trs_FreeModel(game->islandModel);
    trs_FreeModel(game->flagModel);
    SDL_DestroyTexture(game->compassTex);
    SDL_DestroyTexture(game->hintTex);
}

// Returns false if the game should quit
bool gameUpdate(GameState *game) {
    if (game->state == GAME_ROOM_GAME) {
        if (!levelUpdate(game)) {
            menuStart(game, &game->menu);
            game->state = GAME_ROOM_MENU;
        }
    } else if (game->state == GAME_ROOM_MENU) {
        if (!menuUpdate(game, &game->menu)) {
            levelCreate(game);
            game->state = GAME_ROOM_GAME;
        }
    }

    return true;
}

void gameDraw(GameState *game) {
    if (game->state == GAME_ROOM_GAME) {
        levelDraw(game);
    } else if (game->state == GAME_ROOM_MENU) {
        menuDraw(game, &game->menu);
    }
}

void gameUI(GameState *game) {
    trs_Camera *camera = trs_GetCamera();
    if (game->state == GAME_ROOM_GAME) {
        levelDrawUI(game);
    } else if (game->state == GAME_ROOM_MENU) {
        menuDrawUI(game, &game->menu);
    }

    // Debug
    trs_DrawFont(game->font, 1, 0, "FPS: %0.2f\nTriangles: %i", game->fps, trs_GetTriangleCount());
}

SaveLevelInfo *gameSaveGetScores(GameState *game, const char *levelName) {
    // TODO: This
}

bool gameSaveSetScores(GameState *game, const char *levelName, float *checkpointTimes, int checkpointCount, float time) {
    // TODO: This
}