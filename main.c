#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "Software3D.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

typedef struct GameState_t {
    SDL_Renderer *renderer;
    bool *keyboard;
    double delta;
    double time;
    trs_Model testModel;
} GameState;

void gameStart(GameState *game) {
    // Setup camera
    trs_Camera *camera = trs_GetCamera();
    camera->eyes[0] = 10;
    camera->eyes[1] = 10;
    camera->eyes[2] = -4;
    camera->rotation = atan2f(camera->eyes[1], camera->eyes[0]) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2], sqrtf(powf(camera->eyes[1], 2) + powf(camera->eyes[0], 2)));

    // Test model
    const float size = 1;
    trs_Vertex v1 = {{-size, -size, 0, 1}};
    trs_Vertex v2 = {{size, -size, 0, 1}};
    trs_Vertex v3 = {{-size, size, 0, 1}};
    trs_Vertex v4 = {{size, -size, 0, 1}};
    trs_Vertex v5 = {{size, size, 0, 1}};
    trs_Vertex v6 = {{-size, size, 0, 1}};
    trs_Vertex v7 = {{-size + 5, -size, 0, 1}};
    trs_Vertex v8 = {{size + 5, -size, 0, 1}};
    trs_Vertex v9 = {{-size + 5, size, 0, 1}};
    trs_Vertex v10 = {{-size + 5, size, -1, 1}};
    trs_Vertex v11 = {{-size + 5, size, 0, 1}};
    trs_Vertex v12 = {{size + 5, size, 0, 1}};
    trs_Vertex vl[] = {v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12};
    game->testModel = trs_CreateModel(vl, 12);
}

// Returns false if the game should quit
bool gameUpdate(GameState *game) {
    // A small test model
    trs_Camera *camera = trs_GetCamera();
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_rotate_z(model, game->time, model);
    trs_DrawModel(game->testModel, model);

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
    const float MOVE_SPEED = 0.02;
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
    const float cameraSpeed = 2.5 * game->delta;

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
    if (speed != 0) {
        camera->eyes[0] -= speed * cos(direction);
        camera->eyes[1] -= speed * sin(direction);
        camera->eyes[2] += speed * tan(directionZ);
    }
   return true;
}

void gameDraw(GameState *game) {
    
}

void gameEnd(GameState *game) {

}

int main(int argc, char *argv[]) {
    // SDL setup
    SDL_Window *window = SDL_CreateWindow(
        "SDL2 3D", 
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
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
    trs_Init(renderer, window, WINDOW_WIDTH / 4, WINDOW_HEIGHT / 4);
    gameStart(&game);

    // Main loop
    bool running = true;
    SDL_Event event;
    while (running) {
        // Event loop
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
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
        SDL_Texture *backbuffer = trs_EndFrame(&width, &height);

        // Draw 3D portion
        SDL_Rect dst = {
            .x = 0,
            .y = 0,
            .w = width * ((float)WINDOW_WIDTH / width),
            .h = height * ((float)WINDOW_HEIGHT / height)
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