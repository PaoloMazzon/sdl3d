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

typedef struct trs_Camera_t {
    vec3 eyes;
    float rotation;
    float rotationZ;
} trs_Camera;


trs_Camera *trs_GetCamera();
trs_TriangleList *trs_GetTriangleList();
void trs_Init(SDL_Renderer *renderer, SDL_Window *window, float logicalWidth, float logicalHeight);
void trs_BeginFrame();
void trs_AddTriangles();
SDL_Texture *trs_EndFrame();
void trs_End();