#include <SDL2/SDL.h>
#include "Software3D.h"
#include "Game.h"
#include "Player.h"

//******************************** Helpers ********************************//
static void cameraControls(GameState *game) {
    trs_Camera *camera = trs_GetCamera();
    static float cameraLookAngle = GLM_PI / 4;

    // Move the camera around the player
    const float lookSpeed = 2;
    if (game->keyboard[SDL_SCANCODE_A] && !game->keyboardPrevious[SDL_SCANCODE_A]) {
        cameraLookAngle += GLM_PI / 2;//game->delta * lookSpeed;
    }
    if (game->keyboard[SDL_SCANCODE_D] && !game->keyboardPrevious[SDL_SCANCODE_D]) {
        cameraLookAngle -= GLM_PI / 2;//game->delta * lookSpeed;
    }

    // Follow the player
    float cameraX = game->player.x - (12 * cos(cameraLookAngle));
    float cameraY = game->player.y - (12 * sin(cameraLookAngle));
    camera->eyes[0] += ((cameraX) - camera->eyes[0]) * 4 * game->delta;
    camera->eyes[1] += ((cameraY) - camera->eyes[1]) * 4 * game->delta;
    camera->eyes[2] += ((game->player.z + 8) - camera->eyes[2]) * 4 * game->delta;
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));
}

//******************************** Game functions ********************************//

void gameStart(GameState *game) {
    // Setup camera
    trs_Camera *camera = trs_GetCamera();
    camera->eyes[0] = (game->player.x - 8);
    camera->eyes[1] = (game->player.y - 8);
    camera->eyes[2] = (game->player.z + 8);
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));

    // Basic game assets
    game->font = trs_LoadFont("font.png", 7, 8);
    game->compassTex = trs_LoadPNG("compass.png");
    game->playerModel = trs_LoadModel("player.obj");
    game->cubeModel = trs_LoadModel("cube.obj");

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
    playerCreate(game, &game->player);

    // Test hitboxes
    game->cubeHitbox = trs_CalcHitbox(game->cubeModel);
}

void gameEnd(GameState *game) {
    playerDestroy(game, &game->player);
    trs_FreeFont(game->font);
    trs_FreeModel(game->testModel);
    trs_FreeModel(game->playerModel);
    trs_FreeModel(game->groundPlane);
    trs_FreeModel(game->cubeModel);
    trs_FreeHitbox(game->cubeHitbox);
    SDL_DestroyTexture(game->compassTex);
}

// Returns false if the game should quit
bool gameUpdate(GameState *game) {
    cameraControls(game);
    playerUpdate(game, &game->player);

    return true;
}

void gameDraw(GameState *game) {
    // Draw some ground
    const float z = -1;
    trs_DrawModelExt(game->groundPlane, -4, -4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, -4, 0, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, -4, 4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 0, -4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 0, 0, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 0, 4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 4, -4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 4, 0, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 4, 4, z, 1, 1, 1, 0, 0, 0);

    trs_DrawModelExt(game->cubeModel, 2, 2, 0, 1, 1, 1, 0, 0, 0);
    
    playerDraw(game, &game->player);
}

void gameUI(GameState *game) {
    trs_Camera *camera = trs_GetCamera();

    // Draw position
    trs_DrawFont(game->font, 1, 0, "Triangles: %i\nx: %0.2f\ny: %0.2f\nz: %0.2f", trs_GetTriangleCount(), camera->eyes[0], camera->eyes[1], camera->eyes[2]);

    // Draw test hit
    if (trs_Collision(game->cubeHitbox, 2, 2, 0, game->player.hitbox, game->player.x, game->player.y, game->player.z)) {
        trs_DrawFont(game->font, 1, 8 * 4, "Hit");
    }

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
    const float horiX = (cosf(camera->rotation) * 10);
    const float horiY = (sinf(camera->rotation) * 10);
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(game->renderer, startX, startY, startX + horiX, startY + horiY);

    // Vertical orientation
    const float verticalPercent = (camera->rotationZ / GLM_PI);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 255, 255);
    SDL_RenderDrawLine(game->renderer, startX, startY, startX, startY + (verticalPercent * 20));
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderDrawPoint(game->renderer, startX, startY);
}