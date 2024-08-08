#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <time.h>
#include "Software3D.h"
#include "Game.h"

const int WINDOW_WIDTH = 256 * 3;
const int WINDOW_HEIGHT = 224 * 3;

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
    int num;
    uint8_t *keyboard = (void*)SDL_GetKeyboardState(&num);
    GameState game = {
        .renderer = renderer,
        .keyboard = (bool*)keyboard,
        .keyboardPrevious = malloc(num)
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
            game.fps = framerate;
        }

        // Fullscreen
        if (game.keyboard[SDL_SCANCODE_LALT] && game.keyboard[SDL_SCANCODE_RETURN] && !game.keyboardPrevious[SDL_SCANCODE_RETURN]) {
            bool fullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
            SDL_SetWindowFullscreen(window, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        }

        // Copy previous frame
        memcpy(game.keyboardPrevious, game.keyboard, num);
    }

    // Cleanup
    free(game.keyboardPrevious);
    gameEnd(&game);
    trs_End();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}