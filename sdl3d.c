#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <cglm/cglm.h>
#include <stdio.h>

//----------------- CONSTANTS -----------------//
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
#define sgCheckReturn(f) _sgCheckReturn(f, __LINE__)
#define sgCheckMem(f) _sgCheckMem(f, __LINE__)

//----------------- STRUCTS -----------------//
typedef struct sgVertex_t {
    vec4 position;
    vec2 uv;
} sgVertex;

typedef struct sgTriangleList_t {
    sgVertex *vertices;
    SDL_Vertex *verticesSDL;
    int count;
    int size;
} sgTriangleList;

typedef struct sgCamera_t {
    vec3 eyes;
    float rotation;
    float rotationZ;
} sgCamera;

struct sgGameState_t {
    sgTriangleList triangleList;
    SDL_Renderer *renderer;
    sgCamera camera;
    double time;
    double delta; // Difference in time between frames
};

//----------------- TYPES -----------------//
typedef enum {
    SG_RETURN_TYPE_FAILED = -1,
    SG_RETURN_TYPE_SUCCESS = 0,
} SGReturnType;

typedef struct sgGameState_t *sgGameState;

//----------------- UTILITY METHODS -----------------//
void _sgCheckReturn(SGReturnType type, int line) {
    if (type == SG_RETURN_TYPE_FAILED) {
        fprintf(stderr, "Function failed on line %i.\n", line);
        fflush(stderr);
        exit(0);
    }
}

void* _sgCheckMem(void *mem, int line) {
    if (mem == NULL) {
        fprintf(stderr, "Failed to allocate memory on line %i.\n", line);
        fflush(stderr);
        exit(0);
    }
    return mem;
}

//----------------- TRIANGLE LIST METHODS -----------------//
void sgTriangleListReset(sgTriangleList *list) {
    list->count = 0;
    list->size = 0;
    free(list->vertices);
    list->vertices = NULL;
    free(list->verticesSDL);
    list->verticesSDL = NULL;
}

// Guarantees the list has at least this much extra capacity size
void sgTriangleListGuaranteeAdditional(sgTriangleList *list, int size) {
    if (list->size - list->count < size) {
        list->vertices = realloc(list->vertices, sizeof(sgVertex) * (list->size + (size * 2)));
        sgCheckMem(list->vertices);
        list->verticesSDL = realloc(list->verticesSDL, sizeof(SDL_Vertex) * (list->size + (size * 2)));
        sgCheckMem(list->verticesSDL);
        list->size += size * 2;
    }
}

// Adds an object (a bunch of triangles) to a triangle list, multiplying each position by a model matrix
void sgTriangleListAddObject(sgTriangleList *list, sgVertex *vertices, int count, mat4 model) {
    sgTriangleListGuaranteeAdditional(list, count);
    
    // Copy new ones over while multiplying by model matrix
    for (int i = 0; i < count; i++) {
        list->vertices[list->count + i] = vertices[i];
        glm_mat4_mulv(model, list->vertices[list->count + i].position, list->vertices[list->count + i].position);
    }

    list->count += count;
}

//----------------- GAME METHODS -----------------//
SGReturnType sgGameStart(sgGameState state) {

}

// Called before 3D geometry is drawn
SGReturnType sgGameUpdate(sgGameState state) {
    SDL_Rect rect = {
        .x = ((WINDOW_WIDTH / 2) + (cos(state->time) * 100)) - 15,
        .y = ((WINDOW_HEIGHT / 2) + (sin(state->time) * 100)) - 15,
        .w = 30,
        .h = 30
    };
    SDL_SetRenderDrawColor(state->renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(state->renderer, &rect);
}

// Called after 3D geometry is drawn
SGReturnType sgGameDraw(sgGameState state) {

}

SGReturnType sgGameEnd(sgGameState state) {

}

//----------------- MAIN -----------------//
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

    // Timekeeping
    Uint64 startTicks = SDL_GetPerformanceCounter();
    Uint64 startOfSecond = SDL_GetPerformanceCounter();
    double framerate = 0;
    double frameCount = 0;

    // Rendering
    mat4 perspective;
    mat4 view;
    glm_perspective(1.22, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1, 10, perspective);

    // Initialize game
    sgGameState state = sgCheckMem(calloc(1, sizeof(struct sgGameState_t)));
    state->renderer = renderer;
    sgCheckReturn(sgGameStart(state));

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
        state->delta = ((double)(SDL_GetPerformanceCounter() - startTicks) / (double)SDL_GetPerformanceFrequency()) - state->time;
        state->time = (double)(SDL_GetPerformanceCounter() - startTicks) / (double)SDL_GetPerformanceFrequency();
        sgCheckReturn(sgGameUpdate(state));

        // Setup view matrix
        vec3 center = {
            state->camera.eyes[0] + cos(state->camera.rotation),
            state->camera.eyes[1] + sin(state->camera.rotation),
            state->camera.eyes[2] + tan(state->camera.rotationZ)
        };
        vec3 up = {0, 0, 1};
        glm_lookat(state->camera.eyes, center, up, view);

        // Convert triangle list to SDL_Vertex list
        vec4 pos;
        for (int i = 0; i < state->triangleList.count; i++) {
            glm_mat4_mulv(view, state->triangleList.vertices[i].position, pos);
            glm_mat4_mulv(perspective, pos, pos);
            state->triangleList.verticesSDL[i].position.x = (WINDOW_WIDTH / 2) + (pos[0] * (WINDOW_WIDTH / 2));
            state->triangleList.verticesSDL[i].position.x = (WINDOW_HEIGHT / 2) + (pos[1] * (WINDOW_HEIGHT / 2));
            state->triangleList.verticesSDL[i].tex_coord.x = state->triangleList.vertices[i].uv[0];
            state->triangleList.verticesSDL[i].tex_coord.y = state->triangleList.vertices[i].uv[1];
            state->triangleList.verticesSDL[i].color.r = 0;
            state->triangleList.verticesSDL[i].color.r = 0;
            state->triangleList.verticesSDL[i].color.r = 0;
            state->triangleList.verticesSDL[i].color.a = 255;
        }

        // Present the triangle list
        SDL_RenderGeometry(renderer, NULL, state->triangleList.verticesSDL, state->triangleList.count, NULL, 0);
        sgCheckReturn(sgGameDraw(state));

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
    sgCheckReturn(sgGameEnd(state));
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    return 0;
}