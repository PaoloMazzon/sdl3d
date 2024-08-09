#include <SDL2/SDL.h>
#include "Software3D.h"
#include "Game.h"
#include "Level.h"
#include "Player.h"
#include "Menu.h"

//******************************** Game functions ********************************//

void gameStart(GameState *game) {
    // Basic game assets
    game->font = trs_LoadFont("font.png", 7, 8);
    game->menuFont = trs_LoadFont("font2.png", 16, 16);
    game->compassTex = trs_LoadPNG("compass.png");
    game->playerModel = trs_LoadModel("player.obj");
    game->platformModel = trs_LoadModel("platform.obj");
    game->islandModel = trs_LoadModel("island.obj");
    game->hintTex = trs_LoadPNG("hint.png");

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
    levelDestroy(game);
    playerDestroy(game, &game->player);
    trs_FreeFont(game->font);
    trs_FreeFont(game->menuFont);
    trs_FreeModel(game->testModel);
    trs_FreeModel(game->groundPlane);
    trs_FreeModel(game->platformModel);
    trs_FreeModel(game->islandModel);
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
    trs_DrawFont(game->font, 1, 0, "FPS: %0.2f\nTriangles: %i\nx: %0.2f\ny: %0.2f\nz: %0.2f", game->fps, trs_GetTriangleCount(), camera->eyes[0], camera->eyes[1], camera->eyes[2]);
}