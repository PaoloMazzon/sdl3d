#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "Software3D.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

typedef struct GameState_t {
    SDL_Renderer *renderer;
    double delta;
    double time;
} GameState;

void gameStart(GameState *game) {
    // Setup camera
    /*state->camera.eyes[0] = -10;
    state->camera.eyes[1] = 5;
    state->camera.eyes[2] = 4;
    state->camera.rotation = 3.141592635 / 4;
    state->camera.rotationZ = -0.278;*/
}

// Returns false if the game should quit
bool gameUpdate(GameState *game) {
    // A small test model
    /*const float size = 1;
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
    mat4 model = {0};
    glm_mat4_identity(model);
    glm_rotate_z(model, state->time, model);
    trs_TriangleListAddObject(&state->triangleList, vl, 12, model);

    // Look around
    int x, y;
    SDL_GetRelativeMouseState(&x, &y);
    state->camera.rotation -= (float)x * 0.0005;
    state->camera.rotationZ += (float)y * 0.0005;
    state->camera.rotationZ = clamp(state->camera.rotationZ, (-3.14159 / 2) + 0.01, (3.14159 / 2) - 0.01);

    // Move
    float direction = 0;
    float directionZ = 0;
    float speed = 0;
    const float MOVE_SPEED = 0.02;
    float pitch = state->camera.rotationZ;
    float yaw = state->camera.rotation;

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
    const float cameraSpeed = 2.5 * state->delta;

    if (state->keyboard[SDL_SCANCODE_W]) {
        vec3 move;
        glm_vec3_scale(forward, cameraSpeed, move);
        glm_vec3_add(state->camera.eyes, move, state->camera.eyes);
    } else if (state->keyboard[SDL_SCANCODE_A]) {
        vec3 move;
        glm_vec3_scale(right, -cameraSpeed, move);
        glm_vec3_add(state->camera.eyes, move, state->camera.eyes);
    } else if (state->keyboard[SDL_SCANCODE_D]) {
        vec3 move;
        glm_vec3_scale(right, cameraSpeed, move);
        glm_vec3_add(state->camera.eyes, move, state->camera.eyes);
    } else if (state->keyboard[SDL_SCANCODE_S]) {
        vec3 move;
        glm_vec3_scale(forward, -cameraSpeed, move);
        glm_vec3_add(state->camera.eyes, move, state->camera.eyes);
    }
    if (speed != 0) {
        state->camera.eyes[0] -= speed * cos(direction);
        state->camera.eyes[1] -= speed * sin(direction);
        state->camera.eyes[2] += speed * tan(directionZ);
    }*/
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

    // Debug
    const uint8_t colours[] = {
        (uint8_t)(random() * 255),
        (uint8_t)(random() * 255),
        (uint8_t)(random() * 255)
    };

    // Rendering
    mat4 perspective = GLM_MAT4_IDENTITY_INIT;
    glm_perspective(glm_rad(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 100, perspective);

    // Initialize game
    uint8_t *keyboard = (void*)SDL_GetKeyboardState(NULL);
    GameState game = {
        .renderer = renderer
    };
    trs_Init(renderer, window, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
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
        
        if (!gameUpdate(&game)) {
            running = false;
        }

        trs_BeginFrame();
        gameDraw(&game);
        trs_EndFrame();

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