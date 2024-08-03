#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "Software3D.h"

//----------------- CONSTANTS -----------------//
#define trs_CheckReturn(f) _trs_CheckReturn(f, __LINE__)
#define trs_CheckMem(f) _trs_CheckMem(f, __LINE__)
#define trs_CheckSDL(f) _trs_CheckSDL(f, __LINE__)

//----------------- STRUCTS -----------------//

struct trs_GameState_t {
    trs_TriangleList triangleList;
    SDL_Renderer *renderer;
    trs_Camera camera;
    bool *keyboard;
    double time;
    double delta; // Difference in time between frames
    mat4 perspective; // Perspective matrix
    SDL_Window *window;
    SDL_Texture *target;
    float logicalWidth;
    float logicalHeight;
};

typedef struct trs_GameState_t *trs_GameState;
static trs_GameState gGameState;

//----------------- UTILITY METHODS -----------------//
void _trs_CheckReturn(trs_ReturnType type, int line) {
    if (type == TRS_RETURN_TYPE_FAILED) {
        fprintf(stderr, "Function failed on line %i.\n", line);
        fflush(stderr);
        exit(0);
    }
}

void* _trs_CheckMem(void *mem, int line) {
    if (mem == NULL) {
        fprintf(stderr, "Failed to allocate memory on line %i.\n", line);
        fflush(stderr);
        exit(0);
    }
    return mem;
}

void _trs_CheckSDL(void *sdl, int line) {
    if (sdl == NULL) {
        fprintf(stderr, "SDL failed on line %i, SDL error \"%s\".\n", line, SDL_GetError());
        fflush(stderr);
        exit(0);
    }
}

float clamp(float val, float min, float max) {
    return val < min ? min : (val > max ? max : val);
}

float random() {
    return (float)(rand() % 10000) / 10000.0f;
}

//----------------- TRIANGLE LIST METHODS -----------------//
void trs_TriangleListEmpty(trs_TriangleList *list) {
    list->count = 0;
    list->size = 0;
    free(list->vertices);
    list->vertices = NULL;
    free(list->verticesSDL);
    list->verticesSDL = NULL;
}

void trs_TriangleListReset(trs_TriangleList *list) {
    list->count = 0;
}

// Guarantees the list has at least this much extra capacity size
void trs_TriangleListGuaranteeAdditional(trs_TriangleList *list, int size) {
    if (list->size - list->count < size) {
        list->vertices = realloc(list->vertices, sizeof(trs_Vertex) * (list->size + (size * 2)));
        trs_CheckMem(list->vertices);
        list->verticesSDL = realloc(list->verticesSDL, sizeof(SDL_Vertex) * (list->size + (size * 2)));
        trs_CheckMem(list->verticesSDL);
        list->size += size * 2;
    }
}

// Adds an object (a bunch of triangles) to a triangle list, multiplying each position by a model matrix
void trs_TriangleListAddObject(trs_TriangleList *list, trs_Vertex *vertices, int count, mat4 model) {
    trs_TriangleListGuaranteeAdditional(list, count);
    
    // Copy new ones over while multiplying by model matrix
    for (int i = 0; i < count; i++) {
        list->vertices[list->count + i] = vertices[i];
        glm_mat4_mulv(model, list->vertices[list->count + i].position, list->vertices[list->count + i].position);
    }

    list->count += count;
}

//----------------- TRS Methods -----------------//

trs_Camera *trs_GetCamera() {
    return &gGameState->camera;
}

trs_TriangleList *trs_GetTriangleList() {
    return &gGameState->triangleList;
}

void trs_Init(SDL_Renderer *renderer, SDL_Window *window, float logicalWidth, float logicalHeight) {
    gGameState = trs_CheckMem(calloc(1, sizeof(struct trs_GameState_t)));
    gGameState->renderer = renderer;
    gGameState->window = window;
    gGameState->logicalWidth = logicalWidth;
    gGameState->logicalHeight = logicalHeight;
    gGameState->target = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, logicalWidth, logicalHeight);
    trs_CheckSDL(gGameState->target);

    glm_mat4_identity(gGameState->perspective);
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    glm_perspective(glm_rad(45.0f), (float)width / (float)height, 0.1, 100, gGameState->perspective);
}

void trs_BeginFrame() {

}

void trs_AddTriangles() {

}

SDL_Texture *trs_EndFrame(float *width, float *height) {

    // Setup view matrix
    vec3 dir = {
        cos(gGameState->camera.rotationZ) * cos(gGameState->camera.rotation),
        cos(gGameState->camera.rotationZ) * sin(gGameState->camera.rotation),
        sin(gGameState->camera.rotationZ)
    };
    vec3 center = {0};
    glm_vec3_add(gGameState->camera.eyes, dir, center);
    vec3 up = {0, 0, 1};
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_lookat(gGameState->camera.eyes, center, up, view);

    // Convert triangle list to SDL_Vertex list
    mat4 vp = GLM_MAT4_IDENTITY_INIT;
    vec4 pos = {0, 0, 0, 1};
    glm_mat4_mul(gGameState->perspective, view, vp);
    for (int i = 0; i < gGameState->triangleList.count; i++) {
        //glm_mat4_mulv(vp, state->triangleList.vertices[i].position, pos);
        vec4 vertex_pos = {gGameState->triangleList.vertices[i].position[0], gGameState->triangleList.vertices[i].position[1], gGameState->triangleList.vertices[i].position[2], 1.0f};
        glm_mat4_mulv(vp, vertex_pos, pos);
        gGameState->triangleList.verticesSDL[i].position.x = (gGameState->logicalWidth / 2) + ((pos[0] / pos[3]) * (gGameState->logicalWidth / 2));
        gGameState->triangleList.verticesSDL[i].position.y = (gGameState->logicalHeight / 2) + ((pos[1] / pos[3]) * (gGameState->logicalHeight / 2));
        gGameState->triangleList.verticesSDL[i].tex_coord.x = gGameState->triangleList.vertices[i].uv[0];
        gGameState->triangleList.verticesSDL[i].tex_coord.y = gGameState->triangleList.vertices[i].uv[1];
        gGameState->triangleList.verticesSDL[i].color.r = 0;
        gGameState->triangleList.verticesSDL[i].color.g = 0;
        gGameState->triangleList.verticesSDL[i].color.b = 0;
        gGameState->triangleList.verticesSDL[i].color.a = 255;
    }

    // Present the triangle list
    SDL_SetRenderTarget(gGameState->renderer, gGameState->target);
    SDL_SetRenderDrawColor(gGameState->renderer, 255, 255, 255, 255);
    SDL_RenderClear(gGameState->renderer);
    SDL_RenderGeometry(gGameState->renderer, NULL, gGameState->triangleList.verticesSDL, gGameState->triangleList.count, NULL, 0);
    SDL_SetRenderTarget(gGameState->renderer, NULL);
    
    // Return the texture
    if (width != NULL)
        *width = gGameState->logicalWidth;
    if (height != NULL)
        *height = gGameState->logicalHeight;
    return gGameState->target;
}

void trs_End() {
    trs_TriangleListEmpty(&gGameState->triangleList);
}
