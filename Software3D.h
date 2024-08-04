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

typedef struct trs_Model_t {
    trs_Vertex *vertices;
    int count;
};
typedef struct trs_Model_t *trs_Model;

typedef struct trs_Camera_t {
    vec3 eyes;
    float rotation;
    float rotationZ;
} trs_Camera;

void trs_TriangleListEmpty(trs_TriangleList *list);
void trs_TriangleListReset(trs_TriangleList *list);
void trs_TriangleListGuaranteeAdditional(trs_TriangleList *list, int size);
void trs_TriangleListAddObject(trs_TriangleList *list, trs_Vertex *vertices, int count, mat4 model);
float clamp(float val, float min, float max);
float random();
trs_Camera *trs_GetCamera();
trs_TriangleList *trs_GetTriangleList();
void trs_Init(SDL_Renderer *renderer, SDL_Window *window, float logicalWidth, float logicalHeight);
void trs_BeginFrame();
trs_Model trs_CreateModel(trs_Vertex *vertices, int count); // the vertex list will be copied
void trs_DrawModel(trs_Model model, mat4 modelMatrix);
void trs_FreeModel(trs_Model model);
SDL_Texture *trs_EndFrame();
void trs_End();