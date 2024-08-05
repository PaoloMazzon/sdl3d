#define STB_IMAGE_IMPLEMENTATION
#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include "stb_image.h"
#include "tinyobj_loader_c.h"
#include "Software3D.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
Uint32 rmask = 0xff000000;
Uint32 gmask = 0x00ff0000;
Uint32 bmask = 0x0000ff00;
Uint32 amask = 0x000000ff;
#else
Uint32 rmask = 0x000000ff;
Uint32 gmask = 0x0000ff00;
Uint32 bmask = 0x00ff0000;
Uint32 amask = 0xff000000;
#endif

//----------------- CONSTANTS -----------------//
#define trs_CheckReturn(f) _trs_CheckReturn(f, __LINE__)
#define trs_Assert(f) _trs_Assert(f, __LINE__)
#define trs_CheckMem(f) _trs_CheckMem(f, __LINE__)
#define trs_CheckSDL(f) _trs_CheckSDL(f, __LINE__)

//----------------- STRUCTS -----------------//

struct trs_GameState_t {
    trs_TriangleList triangleList;
    trs_TriangleList backbuffer; // for processing on the backend
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
    SDL_Texture *uvtexture; // texture all the models will pull from, "textures.png"
};

typedef struct trs_GameState_t *trs_GameState;

// Globals
static trs_GameState gGameState;
static int gTriangleCount;
static int gTinyOBJSize;
static void *gTinyOBJBuffer;

typedef struct trs_TriangleDepth_t {
    int index;
    float averageDepth;
} trs_TriangleDepth;

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

void _trs_Assert(bool val, int line) {
    if (val == false) {
        fprintf(stderr, "Assert failed on line %i.\n", line);
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

SDL_Texture *trs_LoadPNG(const char *filename) {
    SDL_Texture *out;
    int imageW, imageH, comp;
    void *pixels = stbi_load(filename, &imageW, &imageH, &comp, 4);
    trs_Assert(pixels != NULL);
    SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(pixels, imageW, imageH, 32, 4 * imageW, rmask, gmask, bmask, amask);
    trs_CheckSDL(surf);
    out = SDL_CreateTextureFromSurface(gGameState->renderer, surf);
    trs_CheckSDL(out);
    SDL_FreeSurface(surf);
    stbi_image_free(pixels);
    return out;
}

static uint8_t *trs_LoadFile(const char *filename, int *size) {
    FILE* file = fopen(filename, "rb");
    trs_Assert(file != NULL);
	unsigned char *buffer = NULL;
	*size = 0;

    // Find file size
    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    buffer = trs_CheckMem(malloc(*size));
    
    // Fill the buffer
    fread(buffer, 1, *size, file);
    fclose(file);

	return buffer;
}

static void trs_GetTinyOBJFileData(void* ctx, const char* filename, const int is_mtl, const char* obj_filename, char** data, size_t* len) {
    *data = gTinyOBJBuffer;
    *len = gTinyOBJSize;
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
    trs_Assert(size % 3 == 0);
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

//----------------- Model Methods -----------------//

trs_Model trs_LoadModel(const char *filename) {
    // Load obj - from tinyobj viewer example
    gTinyOBJBuffer = trs_LoadFile(filename, &gTinyOBJSize);
    tinyobj_attrib_t attrib;
    tinyobj_shape_t* shapes = NULL;
    size_t num_shapes;
    tinyobj_material_t* materials = NULL;
    size_t num_materials;
    unsigned int flags = TINYOBJ_FLAG_TRIANGULATE;
    int ret = tinyobj_parse_obj(&attrib, &shapes, &num_shapes, &materials, &num_materials, "asd", trs_GetTinyOBJFileData, NULL, flags);
    trs_Assert(ret == TINYOBJ_SUCCESS);

    // Allocate
    trs_Vertex *newVertices = trs_CheckMem(calloc(attrib.num_faces * 3, sizeof(trs_Vertex)));
    trs_Model model = trs_CheckMem(malloc(sizeof(struct trs_Model_t)));
    model->count = attrib.num_faces * 3;
    model->vertices = newVertices;
    int currentVertex = 0;

    // Parse vertices
    for (int faceIndex = 0; faceIndex < attrib.num_faces; faceIndex++) {
        trs_Vertex vertex = {0};
        const int vertexIndex = attrib.faces[faceIndex].v_idx;
		const int textureIndex = attrib.faces[faceIndex].vt_idx;
        vertex.position[0] = attrib.vertices[(vertexIndex * 3) + 0];
        vertex.position[1] = attrib.vertices[(vertexIndex * 3) + 1];
        vertex.position[2] = attrib.vertices[(vertexIndex * 3) + 2];
        vertex.position[3] = 1;
        vertex.uv[0] = attrib.texcoords[(textureIndex * 2) + 0];
        vertex.uv[1] = 1 - attrib.texcoords[(textureIndex * 2) + 1];
        newVertices[currentVertex++] = vertex;
    }

    // Free tinyobj
    tinyobj_attrib_free(&attrib);
    tinyobj_shapes_free(shapes, num_shapes);
    tinyobj_materials_free(materials, num_materials);

    return model;
}

trs_Model trs_CreateModel(trs_Vertex *vertices, int count) {
    trs_Vertex *newVertices = trs_CheckMem(malloc(sizeof(trs_Vertex) * count));
    trs_Model model = trs_CheckMem(malloc(sizeof(struct trs_Model_t)));
    model->count = count;
    model->vertices = newVertices;

    // Copy the vertices
    for (int i = 0; i < count; i++) {
        newVertices[i] = vertices[i];
    }

    return model;
}

void trs_DrawModel(trs_Model model, mat4 modelMatrix) {
    trs_TriangleListGuaranteeAdditional(&gGameState->triangleList, model->count);
    
    // Copy new ones over while multiplying by model matrix
    for (int i = 0; i < model->count; i++) {
        gGameState->triangleList.vertices[gGameState->triangleList.count + i] = model->vertices[i];
        glm_mat4_mulv(modelMatrix, gGameState->triangleList.vertices[gGameState->triangleList.count + i].position, gGameState->triangleList.vertices[gGameState->triangleList.count + i].position);
    }

    gGameState->triangleList.count += model->count;
}

void trs_FreeModel(trs_Model model) {
    if (model != NULL) {
        free(model->vertices);
        free(model);
    }
}

//----------------- Font Methods -----------------//

trs_Font trs_LoadFont(const char *filename, int w, int h) {
    trs_Font font = trs_CheckMem(malloc(sizeof(struct trs_Font_t)));
    font->w = w;
    font->h = h;
    font->bitmap = trs_LoadPNG(filename);
    return font;
}

void trs_DrawFont(trs_Font font, float x, float y, const char *fmt, ...) {
    float horizontal = x;
    int width, height;
    
    // Deal with varargs
    char buffer[1024];
    va_list list;
    va_start(list, fmt);
    vsnprintf(buffer, 1024, fmt, list);
    va_end(list);
    char *string = buffer;

    SDL_QueryTexture(font->bitmap, NULL, NULL, &width, &height);
    while (*string != 0) {
        if (*string == 32) { // space
            horizontal += font->w;
        } else if (*string == '\n') { // newline
            horizontal = x;
            y += font->h;
        } else if (*string > 32 && *string < 128) { // normal character
            SDL_Rect src = {
                .x = ((*string - 32) * font->w) % width,
                .y = ((int)((*string - 32) * font->w) / width) * font->h,
                .w = font->w,
                .h = font->h
            };
            SDL_Rect dst = {
                .x = horizontal,
                .y = y,
                .w = font->w,
                .h = font->h
            };
            SDL_RenderCopy(gGameState->renderer, font->bitmap, &src, &dst);
            horizontal += font->w;
        }
        string++;
    }
}

void trs_FreeFont(trs_Font font) {
    if (font != NULL) {
        SDL_DestroyTexture(font->bitmap);
        free(font);
    }
}

//----------------- Main Methods -----------------//

trs_Camera *trs_GetCamera() {
    return &gGameState->camera;
}

void trs_Init(SDL_Renderer *renderer, SDL_Window *window, float logicalWidth, float logicalHeight) {
    // Create the game state and target texture
    gGameState = trs_CheckMem(calloc(1, sizeof(struct trs_GameState_t)));
    gGameState->renderer = renderer;
    gGameState->window = window;
    gGameState->logicalWidth = logicalWidth;
    gGameState->logicalHeight = logicalHeight;
    gGameState->target = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window), SDL_TEXTUREACCESS_TARGET, logicalWidth, logicalHeight);
    trs_CheckSDL(gGameState->target);

    // Perspective matrix
    glm_mat4_identity(gGameState->perspective);
    glm_perspective(glm_rad(45.0f), logicalWidth / logicalHeight, 0.1, 100, gGameState->perspective);

    // Load uv texture
    gGameState->uvtexture = trs_LoadPNG("textures.png");
}

void trs_BeginFrame() {
    trs_TriangleListReset(&gGameState->triangleList);
}

bool trs_InFrustrum(vec4 *frustum, vec4 point) {
    for (int i = 0; i < 6; i++) {
        if (glm_vec4_dot(frustum[i], point) < 0) {
            return false;
        }
    }
    return true;
}

// Goes through the triangle list and builds the backbuffer with all the triangles within the camera frustrum
void trs_FrustumCull(mat4 viewproj) {
    // Make sure the backbuffer can handle the potential new triangles
    trs_TriangleListGuaranteeAdditional(&gGameState->backbuffer, gGameState->triangleList.count);
    trs_TriangleListReset(&gGameState->backbuffer);

    // Get and normalize planes
    vec4 planes[6];
    glm_frustum_planes(viewproj, planes);
    for (int i = 0; i < 6; i++)
        glm_plane_normalize(planes[i]);
    
    // Loop through the whole triangle list and add any triangles to the backbuffer that have
    // at least one vertex in the frustrum
    bool triangleMakesCut = false;
    int i;
    for (i = 0; i < gGameState->triangleList.count; i++) {
        // New triangle, check if the old makes the cut
        if (i != 0 && i % 3 == 0 && triangleMakesCut) {
            triangleMakesCut = false;
            const int startingVertex = ((i / 3) - 1) * 3;
            gGameState->backbuffer.vertices[gGameState->backbuffer.count] = gGameState->triangleList.vertices[startingVertex];
            gGameState->backbuffer.vertices[gGameState->backbuffer.count + 1] = gGameState->triangleList.vertices[startingVertex + 1];
            gGameState->backbuffer.vertices[gGameState->backbuffer.count + 2] = gGameState->triangleList.vertices[startingVertex + 2];
            gGameState->backbuffer.count += 3;
        }

        // Check if this triangle is allowed
        if (trs_InFrustrum(planes, gGameState->triangleList.vertices[i].position)) {
            triangleMakesCut = true;
        }
    }
    if (triangleMakesCut) {
        triangleMakesCut = false;
        const int startingVertex = ((i / 3) - 1) * 3;
        gGameState->backbuffer.vertices[gGameState->backbuffer.count] = gGameState->triangleList.vertices[startingVertex];
        gGameState->backbuffer.vertices[gGameState->backbuffer.count + 1] = gGameState->triangleList.vertices[startingVertex + 1];
        gGameState->backbuffer.vertices[gGameState->backbuffer.count + 2] = gGameState->triangleList.vertices[startingVertex + 2];
        gGameState->backbuffer.count += 3;
    }
}

// Comparison function for the quick sort
int comp(const void *val1, const void *val2) {
    const trs_TriangleDepth *triangle1 = val1;
    const trs_TriangleDepth *triangle2 = val2;
    return triangle1->averageDepth > triangle2->averageDepth ? -1 : 1;
}

// Resets the front buffer and builds it back from the backbuffer in order of the painters algorithm
void trs_PaintersAlgorithm() {
    // Make sure the front buffer is of the right soul
    trs_TriangleListReset(&gGameState->triangleList);
    gGameState->triangleList.count = gGameState->backbuffer.count;
    gTriangleCount = gGameState->triangleList.count / 3;

    // Create triangle depth list
    trs_TriangleDepth *triangleDepths = trs_CheckMem(calloc(gGameState->backbuffer.count / 3, sizeof(struct trs_TriangleDepth_t)));
    for (int i = 0; i < gGameState->backbuffer.count / 3; i++) {
        triangleDepths[i].averageDepth = (gGameState->backbuffer.vertices[i * 3].position[2] + gGameState->backbuffer.vertices[(i * 3) + 1].position[2] + gGameState->backbuffer.vertices[(i * 3) + 2].position[2]) / 3;
        triangleDepths[i].index = i;
    }

    // Sort
    qsort(triangleDepths, gGameState->backbuffer.count / 3, sizeof(struct trs_TriangleDepth_t), comp);

    // Copy sorted triangles to triangle list
    for (int i = 0; i < gGameState->backbuffer.count / 3; i++) {
        const int tri = triangleDepths[i].index;
        gGameState->triangleList.vertices[i * 3] = gGameState->backbuffer.vertices[tri * 3];
        gGameState->triangleList.vertices[(i * 3) + 1] = gGameState->backbuffer.vertices[(tri * 3) + 1];
        gGameState->triangleList.vertices[(i * 3) + 2] = gGameState->backbuffer.vertices[(tri * 3) + 2];
    }
    free(triangleDepths);
}

SDL_Texture *trs_EndFrame(float *width, float *height, bool resetTarget) {

    // Setup view matrix
    vec3 dir = {
        cos(gGameState->camera.rotation),// * cos(gGameState->camera.rotation),
        sin(gGameState->camera.rotation),// * sin(gGameState->camera.rotation),
        tan(gGameState->camera.rotationZ)
    };
    vec3 center = {0};
    glm_vec3_add(gGameState->camera.eyes, dir, center);
    vec3 up = {0, 0, 1};
    mat4 view = GLM_MAT4_IDENTITY_INIT;
    glm_lookat(gGameState->camera.eyes, center, up, view);

    // Convert triangle list to SDL_Vertex list
    mat4 vp = GLM_MAT4_IDENTITY_INIT;
    glm_mat4_mul(gGameState->perspective, view, vp);

    // Frustrum cull and painters algorithm
    trs_FrustumCull(vp);

    // Calculate MVP per-vertex
    for (int i = 0; i < gGameState->backbuffer.count; i++) {
        glm_mat4_mulv(vp, gGameState->backbuffer.vertices[i].position, gGameState->backbuffer.vertices[i].position);
    }

    // Painters
    trs_PaintersAlgorithm();

    // Compile the SDL vertex list
    for (int i = 0; i < gGameState->triangleList.count; i++) {
        float *pos = gGameState->triangleList.vertices[i].position;
        gGameState->triangleList.verticesSDL[i].position.x = (gGameState->logicalWidth / 2) + ((pos[0] / pos[3]) * (gGameState->logicalWidth / 2));
        gGameState->triangleList.verticesSDL[i].position.y = (gGameState->logicalHeight / 2) + ((pos[1] / pos[3]) * (gGameState->logicalHeight / 2));
        gGameState->triangleList.verticesSDL[i].tex_coord.x = gGameState->triangleList.vertices[i].uv[0];
        gGameState->triangleList.verticesSDL[i].tex_coord.y = gGameState->triangleList.vertices[i].uv[1];
        gGameState->triangleList.verticesSDL[i].color.r = 255;
        gGameState->triangleList.verticesSDL[i].color.g = 255;
        gGameState->triangleList.verticesSDL[i].color.b = 255;
        gGameState->triangleList.verticesSDL[i].color.a = 255;
    }

    // Present the triangle list
    SDL_SetRenderTarget(gGameState->renderer, gGameState->target);
    SDL_SetRenderDrawColor(gGameState->renderer, 255, 255, 255, 255);
    SDL_RenderClear(gGameState->renderer);
    SDL_RenderGeometry(gGameState->renderer, gGameState->uvtexture, gGameState->triangleList.verticesSDL, gGameState->triangleList.count, NULL, 0);
    if (resetTarget)
        SDL_SetRenderTarget(gGameState->renderer, NULL);
    
    // Return the texture
    if (width != NULL)
        *width = gGameState->logicalWidth;
    if (height != NULL)
        *height = gGameState->logicalHeight;
    return gGameState->target;
}

void trs_End() {
    SDL_DestroyTexture(gGameState->uvtexture);
    SDL_DestroyTexture(gGameState->target);
    trs_TriangleListEmpty(&gGameState->triangleList);
}

int trs_GetTriangleCount() {
    return gTriangleCount;
}