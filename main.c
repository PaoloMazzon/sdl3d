#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <time.h>
#include "Software3D.h"

const int WINDOW_WIDTH = 256 * 3;
const int WINDOW_HEIGHT = 224 * 3;

typedef struct Player_t {
    float x, y, z;
    float velocityX, velocityY, velocityZ;
    float speed;
    float direction;
    float drawDirection;
    float jumpTimer;
    float zscale; // for animations
    bool onGroundLastFrame;
    float squishTimer; // for animations
} Player;

typedef struct GameState_t {
    SDL_Renderer *renderer;
    Player player;
    bool *keyboard;
    double delta;
    double time;
    trs_Model testModel;
    trs_Font font;
    SDL_Texture *compassTex;
    trs_Model playerModel;
    trs_Model groundPlane;
} GameState;

//******************************** Player ********************************//
void playerCreate(GameState *game, Player *player) {
    player->x = 0;
    player->y = 0;
    player->z = 0;
    player->onGroundLastFrame = true;
}

bool playerOnGround(GameState *game, Player *player) {
    return player->z <= 0;
}

void playerUpdate(GameState *game, Player *player) {
    trs_Camera *cam = trs_GetCamera();
    const float speed = 0.3 * game->delta;
    const float gravity = 15 * game->delta;
    const float terminalVelocity = -10;
    const float topSpeed = 8;
    const float friction = 0.8 * game->delta;
    const float jumpSpeed = 30;
    const float jumpDuration = 0.8; // in seconds
    const float squishDuration = 0.3;

    // Get player input
    player->velocityX = player->velocityY = player->velocityZ = 0;
    player->velocityX = (float)game->keyboard[SDL_SCANCODE_RIGHT] - (float)game->keyboard[SDL_SCANCODE_LEFT];
    player->velocityY = (float)game->keyboard[SDL_SCANCODE_UP] - (float)game->keyboard[SDL_SCANCODE_DOWN];
    const bool jump = playerOnGround(game, player) && game->keyboard[SDL_SCANCODE_SPACE];

    // Move relative to the camera's placement
    if (player->velocityX != 0 || player->velocityY != 0) {
        player->direction = cam->rotation + atan2f(player->velocityX, player->velocityY);
        player->speed = clamp(player->speed + speed, 0, topSpeed * game->delta);
    } else {
        player->speed = clamp(player->speed - friction, 0, 999);
    }
    const float xComponent = cos(player->direction);
    const float yComponent = sin(player->direction);
    player->x += xComponent * player->speed;
    player->y += yComponent * player->speed;

    // Jump & gravity
    if (jump)
        player->jumpTimer = jumpDuration;
    if (player->jumpTimer > 0) {
        player->jumpTimer = clamp(player->jumpTimer - game->delta, 0, 999);
        player->velocityZ += jumpSpeed * (player->jumpTimer / jumpDuration) * game->delta;
        const float x = (player->jumpTimer / jumpDuration);
        player->zscale = 1 + ((-powf((2 * x) - 1, 2) + 1) * 0.3);
    } else {
        player->zscale = 1;
    }
    player->velocityZ = clamp(player->velocityZ - gravity, terminalVelocity, 9999999);
    player->z = clamp(player->z + (player->velocityZ), 0, 999);

    // Squash animation
    if (playerOnGround(game, player) && !player->onGroundLastFrame) {
        player->squishTimer = squishDuration;
    }
    if (player->squishTimer > 0) {
        const float percent = (player->squishTimer / squishDuration);
        player->squishTimer -= game->delta;
        player->zscale = 1 - ((-powf((2 * percent) - 1, 2) + 1) * 0.8);
    }

    player->onGroundLastFrame = playerOnGround(game, player);
}

static double normalizeAngle(double angle) {
    while (angle < 0) {
        angle += 2 * GLM_PI;
    }
    while (angle >= 2 * GLM_PI) {
        angle -= 2 * GLM_PI;
    }
    return angle;
}

void playerDraw(GameState *game, Player *player) {
    player->drawDirection = normalizeAngle(player->drawDirection);
    player->direction = normalizeAngle(player->direction);
    float difference = (player->direction - player->drawDirection);
    if (difference > GLM_PI) {
        difference -= 2 * GLM_PI;
    } else if (difference < -GLM_PI) {
        difference += 2 * GLM_PI;
    }
    player->drawDirection += difference * 10 * game->delta;
    player->drawDirection = normalizeAngle(player->drawDirection);

    trs_DrawModelExt(game->playerModel, player->x, player->y, player->z, 1, 1, player->zscale, 0, 0, player->drawDirection + (GLM_PI / 2));
}

void playerDestroy(GameState *game, Player *player) {

}

//******************************** Helpers ********************************//
void cameraControls(GameState *game) {
    trs_Camera *camera = trs_GetCamera();

    // Follow the player
    camera->eyes[0] += ((game->player.x - 8) - camera->eyes[0]) * 4 * game->delta;
    camera->eyes[1] += ((game->player.y - 8) - camera->eyes[1]) * 4 * game->delta;
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

    // Player
    playerCreate(game, &game->player);

    // Basic game assets
    game->font = trs_LoadFont("font.png", 7, 8);
    game->compassTex = trs_LoadPNG("compass.png");
    game->playerModel = trs_LoadModel("player.obj");

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
}

void gameEnd(GameState *game) {
    playerDestroy(game, &game->player);
    trs_FreeFont(game->font);
    trs_FreeModel(game->testModel);
    trs_FreeModel(game->playerModel);
    trs_FreeModel(game->groundPlane);
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
    
    playerDraw(game, &game->player);
}

void gameUI(GameState *game) {
    trs_Camera *camera = trs_GetCamera();

    // Draw position
    trs_DrawFont(game->font, 1, 0, "Triangles: %i\nx: %0.2f\ny: %0.2f\nz: %0.2f", trs_GetTriangleCount(), camera->eyes[0], camera->eyes[1], camera->eyes[2]);

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

int main(int argc, char *argv[]) {
    // SDL setup
    SDL_Window *window = SDL_CreateWindow(
        "SDL2 3D", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_FULLSCREEN_DESKTOP
    );
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC
    );
    SDL_SetRelativeMouseMode(true);
    srand(time(NULL));

    // Timekeeping
    Uint64 startTicks = SDL_GetPerformanceCounter();
    Uint64 startOfSecond = SDL_GetPerformanceCounter();
    double framerate = 0;
    double frameCount = 0;

    // Initialize game
    uint8_t *keyboard = (void*)SDL_GetKeyboardState(NULL);
    GameState game = {
        .renderer = renderer,
        .keyboard = (bool*)keyboard
    };
    trs_Init(renderer, window, 256, 224);
    gameStart(&game);

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running) {
        // Event loop
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                running = false;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        // Update game
        game.delta = ((double)(SDL_GetPerformanceCounter() - startTicks) / (double)SDL_GetPerformanceFrequency()) - game.time;
        game.time = (double)(SDL_GetPerformanceCounter() - startTicks) / (double)SDL_GetPerformanceFrequency();

        trs_BeginFrame();
        if (!gameUpdate(&game)) {
            running = false;
        }
        gameDraw(&game);
        float width, height;
        SDL_Texture *backbuffer = trs_EndFrame(&width, &height, false);
        gameUI(&game);
        SDL_SetRenderTarget(renderer, NULL);

        // Draw the internal texture integer scaled
        int windowWidth, windowHeight;
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        float scale = (float)windowHeight / height;
        if (scale > (float)windowWidth / width)
            scale = (float)windowWidth / width;
        scale = floorf(scale);
        SDL_Rect dst = {
            .x = ((float)windowWidth - (width * scale)) * 0.5,
            .y = ((float)windowHeight - (height * scale)) * 0.5,
            .w = width * scale,
            .h = height * scale
        };
        SDL_RenderCopy(renderer, backbuffer, NULL, &dst);

        // End frame
        SDL_RenderPresent(renderer);

        // Timekeeping
        frameCount += 1;
        if (SDL_GetPerformanceCounter() - startOfSecond >= SDL_GetPerformanceFrequency()) {
            framerate = frameCount / ((double)(SDL_GetPerformanceCounter() - startOfSecond) / (double)SDL_GetPerformanceFrequency());
            frameCount = 0;
            startOfSecond = SDL_GetPerformanceCounter();
            char buffer[1000];
            snprintf(buffer, 1000, "SDL2 3D %.2ffps", framerate);
            SDL_SetWindowTitle(window, buffer);
        }
    }

    // Cleanup
    gameEnd(&game);
    trs_End();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}