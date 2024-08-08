#include <SDL2/SDL.h>
#include "Software3D.h"
#include "Game.h"
#include "Level.h"
#include "Player.h"

//******************************** Game functions ********************************//

void gameStart(GameState *game) {
    // Basic game assets
    game->font = trs_LoadFont("font.png", 7, 8);
    game->compassTex = trs_LoadPNG("compass.png");
    game->playerModel = trs_LoadModel("player.obj");
    game->platformModel = trs_LoadModel("platform.obj");
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
    trs_Vertex v1 = {{-size, -size, 0, 1},      { 8 / 128.0f,  0 / 128.0f}};
    trs_Vertex v2 = {{size, -size, 0, 1},       {24 / 128.0f,  0 / 128.0f}};
    trs_Vertex v3 = {{-size, size, 0, 1},       { 8 / 128.0f, 16 / 128.0f}};
    trs_Vertex v4 = {{size, -size, 0, 1},       {24 / 128.0f,  0 / 128.0f}};
    trs_Vertex v5 = {{size, size, 0, 1},        {24 / 128.0f, 16 / 128.0f}};
    trs_Vertex v6 = {{-size, size, 0, 1},       { 8 / 128.0f, 16 / 128.0f}};
    
    trs_Vertex v7 = {{-size, -size, 0, 1},      {24 / 128.0f, 0 / 128.0f}};
    trs_Vertex v8 = {{-size, -size, -size, 1},  {24 / 128.0f, 8 / 128.0f}};
    trs_Vertex v9 = {{-size, size, 0, 1},       { 8 / 128.0f, 0 / 128.0f}};
    trs_Vertex v10 = {{-size, size, 0, 1},      { 8 / 128.0f, 0 / 128.0f}};
    trs_Vertex v11 = {{-size, size, -size, 1},  { 8 / 128.0f, 8 / 128.0f}};
    trs_Vertex v12 = {{-size, -size, -size, 1}, {24 / 128.0f, 8 / 128.0f}};
    
    trs_Vertex v13 = {{-size + 5, -size, 0, 1}, {40 / 128.0f,  0 / 128.0f}};
    trs_Vertex v14 = {{size + 5, -size, 0, 1},  {56 / 128.0f,  0 / 128.0f}};
    trs_Vertex v15 = {{-size + 5, size, 0, 1},  {40 / 128.0f, 16 / 128.0f}};
    
    trs_Vertex v16 = {{-size + 5, size, -1, 1}, {56 / 128.0f,  0 / 128.0f}};
    trs_Vertex v17 = {{-size + 5, size, 0, 1},  {56 / 128.0f,  8 / 128.0f}};
    trs_Vertex v18 = {{size + 5, size, 0, 1},   {72 / 128.0f,  8 / 128.0f}};
    trs_Vertex vl[] = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16, v17, v18};
    game->testModel = trs_CreateModel(vl, 18);
    
    // Player
    levelCreate(game);
    playerCreate(game, &game->player);
}

void gameEnd(GameState *game) {
    levelDestroy(game);
    playerDestroy(game, &game->player);
    trs_FreeFont(game->font);
    trs_FreeModel(game->testModel);
    trs_FreeModel(game->groundPlane);
    trs_FreeModel(game->platformModel);
    SDL_DestroyTexture(game->compassTex);
    SDL_DestroyTexture(game->hintTex);
}

// Returns false if the game should quit
bool gameUpdate(GameState *game) {
    levelUpdate(game);

    return true;
}

void gameDraw(GameState *game) {
    levelDraw(game);
}

void gameUI(GameState *game) {
    trs_Camera *camera = trs_GetCamera();
    levelDrawUI(game);

    // Debug
    trs_DrawFont(game->font, 1, 0, "FPS: %0.2f\nTriangles: %i\nx: %0.2f\ny: %0.2f\nz: %0.2f", game->fps, trs_GetTriangleCount(), camera->eyes[0], camera->eyes[1], camera->eyes[2]);

    // Hint
    SDL_RenderCopy(game->renderer, game->hintTex, NULL, &((SDL_Rect){.x = 0, .y = 205, .w = 81, .h = 19}));

    // Draw orientation
    const float startX = 231;
    const float startY = 23;
    SDL_Rect dst = {
        .x = startX - 13,
        .y = startY - 13,
        .w = 25,
        .h = 25
    };
    SDL_RenderCopy(game->renderer, game->compassTex, NULL, &dst);

    // Horizontal orientation
    const float rotation = camera->rotation - ((3 * GLM_PI) / 4);
    const float horiX = (cosf(rotation) * 10);
    const float horiY = (sinf(rotation) * 10);
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(game->renderer, startX, startY, startX + horiX, startY + horiY);

    // Vertical orientation
    const float verticalPercent = (camera->rotationZ / GLM_PI);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 255, 255);
    SDL_RenderDrawLine(game->renderer, startX, startY, startX, startY - (verticalPercent * 20));
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderDrawPoint(game->renderer, startX, startY);
}