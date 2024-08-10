/// \file Software3D.h
/// \brief Declares the software 3D renderer using SDL_RenderGeometry
#pragma once
#include <cglm/cglm.h>

typedef enum {
    TRS_RETURN_TYPE_FAILED = -1,
    TRS_RETURN_TYPE_SUCCESS = 0,
} trs_ReturnType;

typedef struct trs_Vertex_t {
    vec4 position;
    vec2 uv;
} trs_Vertex;

typedef struct trs_TriangleList_t {
    trs_Vertex *vertices;
    SDL_Vertex *verticesSDL;
    int count;
    int size;
} trs_TriangleList;

typedef struct trs_Camera_t {
    vec3 eyes;
    float rotation;
    float rotationZ;
} trs_Camera;

struct trs_Font_t {
    SDL_Texture *bitmap;
    int w;
    int h;
};
typedef struct trs_Font_t *trs_Font;

struct trs_Hitbox_t {
    vec3 box[2];
};
typedef struct trs_Hitbox_t *trs_Hitbox;
typedef void *trs_Sound;

struct trs_Model_t {
    trs_Vertex *vertices;
    trs_Hitbox hitbox;
    int count;
};
typedef struct trs_Model_t *trs_Model;

// Font
trs_Font trs_LoadFont(const char *filename, int w, int h); // Expects each character to be w*h and ascii 32-128
void trs_DrawFont(trs_Font font, float x, float y, const char *fmt, ...);
void trs_FreeFont(trs_Font font);

// Triangle lists
void trs_TriangleListEmpty(trs_TriangleList *list);
void trs_TriangleListReset(trs_TriangleList *list);
void trs_TriangleListGuaranteeAdditional(trs_TriangleList *list, int size);
void trs_TriangleListAddObject(trs_TriangleList *list, trs_Vertex *vertices, int count, mat4 model);

// Utility
float clamp(float val, float min, float max);
float random();

// Getters
trs_Camera *trs_GetCamera();
int trs_GetTriangleCount();
SDL_Texture *trs_LoadPNG(const char *filename); // shorthand for stb image

// Model loading/drawing
trs_Model trs_CreateModel(trs_Vertex *vertices, int count); // the vertex list will be copied
trs_Model trs_LoadModel(const char *filename); // loads a model from a .obj
void trs_DrawModel(trs_Model model, mat4 modelMatrix);
void trs_DrawModelExt(trs_Model model, float x, float y, float z, float scaleX, float scaleY, float scaleZ, float rotationX, float rotationY, float rotationZ);
void trs_FreeModel(trs_Model model);

// Sounds
trs_Sound trs_LoadSound(const char *filename);
void trs_PlaySound(trs_Sound sound, float volume, bool looping);
void trs_StopAllSound();
void trs_FreeSound(trs_Sound sound);

// AABB hitboxes (slight abstraction over cglm)
trs_Hitbox trs_CreateHitbox(float x1, float y1, float z1, float x2, float y2, float z2);
bool trs_Collision(trs_Hitbox hb1, float x1, float y1, float z1, trs_Hitbox hb2, float x2, float y2, float z2);
trs_Hitbox trs_GetModelHitbox(trs_Model model); // returns the hitbox assiciated with a model
void trs_FreeHitbox(trs_Hitbox hb);

// Core renderer
void trs_Init(SDL_Renderer *renderer, SDL_Window *window, float logicalWidth, float logicalHeight);
void trs_BeginFrame();
SDL_Texture *trs_EndFrame(float *width, float *height, bool resetTarget);
void trs_End();