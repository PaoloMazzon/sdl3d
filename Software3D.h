/// \file Software3D.h
/// \brief Declares the software 3D renderer using SDL_RenderGeometry
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

struct trs_Model_t {
    trs_Vertex *vertices;
    int count;
};
typedef struct trs_Model_t *trs_Model;

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
void trs_FreeModel(trs_Model model);

// Core renderer
void trs_Init(SDL_Renderer *renderer, SDL_Window *window, float logicalWidth, float logicalHeight);
void trs_BeginFrame();
SDL_Texture *trs_EndFrame(float *width, float *height, bool resetTarget);
void trs_End();