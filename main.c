#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <time.h>
#include "Software3D.h"

const int WINDOW_WIDTH = 256 * 3;
const int WINDOW_HEIGHT = 224 * 3;

typedef struct GameState_t {
    SDL_Renderer *renderer;
    bool *keyboard;
    double delta;
    double time;
    trs_Model testModel;
    trs_Model skyboxModel;
    trs_Font font;
    SDL_Texture *compassTex;
} GameState;

void gameStart(GameState *game) {
    // Setup camera
    trs_Camera *camera = trs_GetCamera();
    camera->eyes[0] = 10;
    camera->eyes[1] = 10;
    camera->eyes[2] = -4;
    camera->rotation = atan2f(camera->eyes[1], camera->eyes[0]) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2], sqrtf(powf(camera->eyes[1], 2) + powf(camera->eyes[0], 2)));

    // Basic game assets
    game->font = trs_LoadFont("font.png", 7, 8);
    game->compassTex = trs_LoadPNG("compass.png");

    // Test model
    const float size = 1;
    trs_Vertex v1 = {{-size, -size, 0, 1}, {8, 0}};
    trs_Vertex v2 = {{size, -size, 0, 1}, {24, 0}};
    trs_Vertex v3 = {{-size, size, 0, 1}, {8, 16}};
    trs_Vertex v4 = {{size, -size, 0, 1}, {24, 0}};
    trs_Vertex v5 = {{size, size, 0, 1}, {24, 16}};
    trs_Vertex v6 = {{-size, size, 0, 1}, {8, 16}};
    trs_Vertex v7 = {{-size + 5, -size, 0, 1}, {40, 0}};
    trs_Vertex v8 = {{size + 5, -size, 0, 1}, {56, 0}};
    trs_Vertex v9 = {{-size + 5, size, 0, 1}, {40, 16}};
    trs_Vertex v10 = {{-size + 5, size, -1, 1}, {56, 0}};
    trs_Vertex v11 = {{-size + 5, size, 0, 1}, {56, 16}};
    trs_Vertex v12 = {{size + 5, size, 0, 1}, {72, 16}};
    trs_Vertex vl[] = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12};
    game->testModel = trs_CreateModel(vl, 12);

    // Skybox
    const float skyboxSize = 25;
    trs_Vertex sb1 = {{-skyboxSize, -skyboxSize, skyboxSize, 1}};
    trs_Vertex sb2 = {{skyboxSize, -skyboxSize, skyboxSize, 1}};
    trs_Vertex sb3 = {{-skyboxSize, skyboxSize, skyboxSize, 1}};
    trs_Vertex sb4 = {{skyboxSize, -skyboxSize, skyboxSize, 1}};
    trs_Vertex sb5 = {{skyboxSize, skyboxSize, skyboxSize, 1}};
    trs_Vertex sb6 = {{-skyboxSize, skyboxSize, skyboxSize, 1}};
    trs_Vertex sbl[] = {sb1, sb2, sb3, sb4, sb5, sb6};
    game->skyboxModel = trs_CreateModel(sbl, 6);
}

void gameEnd(GameState *game) {
    trs_FreeFont(game->font);
    trs_FreeModel(game->skyboxModel);
    trs_FreeModel(game->testModel);
    SDL_DestroyTexture(game->compassTex);
}

// Returns false if the game should quit
bool gameUpdate(GameState *game) {
    trs_Camera *camera = trs_GetCamera();

    // Look around
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    camera->rotation -= (float)x * 0.0005;
    camera->rotationZ += (float)y * 0.0005;
    camera->rotationZ = clamp(camera->rotationZ, (-3.14159 / 2) + 0.01, (3.14159 / 2) - 0.01);

    // Move
    float direction = 0;
    float directionZ = 0;
    float speed = 0;
    const float MOVE_SPEED = 0.08;
    float pitch = camera->rotationZ;
    float yaw = camera->rotation;

    vec3 forward = {
        cos(pitch) * cos(yaw),
        cos(pitch) * sin(yaw),
        sin(pitch)
    };

    vec3 right = {
        -sin(yaw),
        cos(yaw),
        0
    };

    vec3 up = {0, 0, 1};
    const float cameraSpeed = 6 * game->delta;

    if (game->keyboard[SDL_SCANCODE_W]) {
        vec3 move;
        glm_vec3_scale(forward, cameraSpeed, move);
        glm_vec3_add(camera->eyes, move, camera->eyes);
    }
    if (game->keyboard[SDL_SCANCODE_D]) {
        vec3 move;
        glm_vec3_scale(right, -cameraSpeed, move);
        glm_vec3_add(camera->eyes, move, camera->eyes);
    } 
    if (game->keyboard[SDL_SCANCODE_A]) {
        vec3 move;
        glm_vec3_scale(right, cameraSpeed, move);
        glm_vec3_add(camera->eyes, move, camera->eyes);
    } 
    if (game->keyboard[SDL_SCANCODE_S]) {
        vec3 move;
        glm_vec3_scale(forward, -cameraSpeed, move);
        glm_vec3_add(camera->eyes, move, camera->eyes);
    }
    if (game->keyboard[SDL_SCANCODE_SPACE]) {
        camera->eyes[2] -= cameraSpeed;
    }
    if (game->keyboard[SDL_SCANCODE_LCTRL]) {
        camera->eyes[2] += cameraSpeed;
    }
    if (speed != 0) {
        camera->eyes[0] -= speed * cos(direction);
        camera->eyes[1] -= speed * sin(direction);
        camera->eyes[2] += speed * tan(directionZ);
    }
   return true;
}

void gameDraw(GameState *game) {
    // A small test model
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_z(model, game->time, model);
    trs_DrawModel(game->testModel, model);
}

void gameUI(GameState *game) {
    trs_Camera *camera = trs_GetCamera();

    // Draw position
    trs_DrawFont(game->font, 1, 0, "x: %0.2f\ny: %0.2f\nz: %0.2f", camera->eyes[0], camera->eyes[1], camera->eyes[2]);

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
        SDL_WINDOW_RESIZABLE
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