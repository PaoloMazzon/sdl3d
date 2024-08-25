#include "cJSON.h"
#include "Software3D.h"
#include "Level.h"
#include "Player.h"

//******************************** Chunks ********************************//
// Chunks are a CHUNK_WIDTH-wide block of game-world assets, every chunk is
// loaded at once and where the player is determines which chunks are accessed
// and checked against for drawing and collisions and the like each frame.
// The chunk at player position +/- 1 chunk are handled each frame.
const float CHUNK_WIDTH = 15.0f;

// Returns the index of the chunk this coordinate would belong to
static int getChunkIndex(float x, float y) {
    //x = floorf(x);
    //y = floorf(y);

    // Chunks only exist along the line y=-x, and they only go out in the direction x<0 y>0
    if (y < x) return 0;
    
    // Casts the coordinate to the line y=-x and find the distance on that line from the origin
    const float distance = sqrtf((powf(x, 2) + powf(y, 2)) - powf((-x - y) / (GLM_SQRT2), 2));
    const int out = (int)floorf(distance / CHUNK_WIDTH);
    return out >= 0 ? out : 0;
}

// Returns the chunk at a given index, creating it if it doesn't exist
static Chunk *getChunkAtIndex(Level *level, int index) {
    if (index < 0) return NULL;
    if (level->chunkCount <= index) {
        // Allocate new chunks up to this point
        int oldSize = level->chunkCount;
        level->chunks = realloc(level->chunks, sizeof(struct Chunk_t) * (index + 1));
        level->chunkCount = index + 1;

        // Zero all the chunks up to the new size
        for (int i = oldSize; i < level->chunkCount; i++) {
            level->chunks[i].wallCount = 0;
            level->chunks[i].walls = NULL;
            level->chunks[i].checkpointCount = 0;
            level->chunks[i].checkpoints = NULL;
        }
    }
    return &level->chunks[index];
}

// Returns a chunk at a given position, creating that chunk if it doesn't yet exist
static Chunk *getChunkAtPosition(Level *level, float x, float y) {
    const int index = getChunkIndex(x, y);
    return getChunkAtIndex(level, index);
}

// Iterators
typedef struct WallIterator_t {
    int chunks[3];
    int chunkCount;
    int chunkIndex;
    int iteratorIndex;
} WallIterator;
typedef struct WallIterator_t CheckpointIterator;

static Wall *getWallsNext(Level *level, WallIterator *iter) {
    for (; iter->chunkIndex < iter->chunkCount; iter->chunkIndex++) {
        Chunk *chunk = &level->chunks[iter->chunks[iter->chunkIndex]];
        if (iter->iteratorIndex < chunk->wallCount) {
            iter->iteratorIndex++;
            return &chunk->walls[iter->iteratorIndex - 1];
        }
        iter->iteratorIndex = 0;
    }
    return NULL;
}

static Checkpoint *getCheckpointNext(Level *level, CheckpointIterator *iter) {
    for (; iter->chunkIndex < iter->chunkCount; iter->chunkIndex++) {
        Chunk *chunk = &level->chunks[iter->chunks[iter->chunkIndex]];
        if (iter->iteratorIndex < chunk->checkpointCount) {
            iter->iteratorIndex++;
            return &chunk->checkpoints[iter->iteratorIndex - 1];
        }
        iter->iteratorIndex = 0;
    }
    return NULL;
}

static Wall *getWallsStart(GameState *game, Level *level, WallIterator *iter) {
    // Get the chunk on the player
    const int chunk = getChunkIndex(game->player.x, game->player.y);
    iter->chunkCount = 0;
    iter->iteratorIndex = 0;
    iter->chunkIndex = 0;

    // Count the chunks in range of the player
    if (chunk < level->chunkCount)
        iter->chunks[iter->chunkCount++] = chunk;
    if (chunk + 1 < level->chunkCount)
        iter->chunks[iter->chunkCount++] = chunk + 1;
    if (chunk != 0 && level->chunkCount > 1)
        iter->chunks[iter->chunkCount++] = chunk - 1;
    return getWallsNext(level, iter);
}

static Checkpoint *getCheckpointsStart(GameState *game, Level *level, CheckpointIterator *iter) {
    // Get the chunk on the player
    const int chunk = getChunkIndex(game->player.x, game->player.y);
    iter->chunkCount = 0;
    iter->iteratorIndex = 0;
    iter->chunkIndex = 0;

    // Count the chunks in range of the player
    if (chunk < level->chunkCount)
        iter->chunks[iter->chunkCount++] = chunk;
    if (chunk + 1 < level->chunkCount)
        iter->chunks[iter->chunkCount++] = chunk + 1;
    if (chunk != 0 && level->chunkCount > 1)
        iter->chunks[iter->chunkCount++] = chunk - 1;
    return getCheckpointNext(level, iter);
}

//******************************** Gameworld things ********************************//
// Adds a wall to the proper chunk
void addWall(Level *level, Wall *wall) {
    // Get the chunk associated with this position
    Chunk *chunk = getChunkAtPosition(level, wall->position[0], wall->position[1]);

    // Search for an open wall slot first
    int spot = -1;
    for (int i = 0; i < chunk->wallCount && spot == -1; i++)
        if (chunk->walls[i].active == false)
            spot = i;
    
    // Extend the list
    if (spot == -1) {
        chunk->walls = realloc(chunk->walls, (chunk->wallCount + 10) * sizeof(struct Wall_t));
        for (int i = chunk->wallCount; i < chunk->wallCount + 10; i++)
            chunk->walls[i].active = false;
        spot = chunk->wallCount;
        chunk->wallCount += 10;
    }

    // Copy the wall
    chunk->walls[spot] = *wall;
    chunk->walls[spot].active = true;
    chunk->walls[spot].startMove[0] = chunk->walls[spot].position[0];
    chunk->walls[spot].startMove[1] = chunk->walls[spot].position[1];
    chunk->walls[spot].startMove[2] = chunk->walls[spot].position[2];
    if (chunk->walls[spot].hitbox == NULL)
        chunk->walls[spot].hitbox = trs_GetModelHitbox(chunk->walls[spot].model);
}

// Adds a checkpoint to the proper chunk
void addCheckpoint(Level *level, Checkpoint *checkpoint) {
    // Get the chunk associated with this position
    Chunk *chunk = getChunkAtPosition(level, checkpoint->position[0], checkpoint->position[1]);

    // Search for an open wall slot first
    int spot = -1;
    for (int i = 0; i < chunk->checkpointCount && spot == -1; i++)
        if (chunk->checkpoints[i].active == false)
            spot = i;
    
    // Extend the list
    if (spot == -1) {
        chunk->checkpoints = realloc(chunk->checkpoints, (chunk->checkpointCount + 10) * sizeof(struct Checkpoint_t));
        for (int i = chunk->checkpointCount; i < chunk->checkpointCount + 10; i++)
            chunk->checkpoints[i].active = false;
        spot = chunk->checkpointCount;
        chunk->checkpointCount += 10;
    }

    // Extend save chunk list

    // Copy the wall
    chunk->checkpoints[spot].playerHit = false;
    chunk->checkpoints[spot].active = true;
    chunk->checkpoints[spot].time = 0;
    chunk->checkpoints[spot].index = level->checkpointID++;
    chunk->checkpoints[spot].position[0] = checkpoint->position[0];
    chunk->checkpoints[spot].position[1] = checkpoint->position[1];
    chunk->checkpoints[spot].position[2] = checkpoint->position[2];
}

void updateCheckpoint(GameState *game, Level *level, Checkpoint *checkpoint) {
    if (!checkpoint->playerHit && glm_vec3_distance2((vec3){game->player.x, game->player.y, game->player.z}, checkpoint->position) < 1) {
        checkpoint->playerHit = true;
        const double time = game->time - level->startTime;
        levelDisplayMessage(game, "Checkpoint %i: =%02d:%02d:%03d", checkpoint->index + 1, (int)(time / 60), (int)(fmodf(time, 60)), (int)(fmodf(time * 1000, 1000)));
        checkpoint->time = time;
    }
    // TODO: Handle the player hitting the last checkpoint
}

void updateWall(GameState *game, Level *level, Wall *wall) {
    wall->time += game->delta * (1 / wall->moveFactor);
    if (wall->time < 1) {    
        wall->velocity[0] = (wall->endMove[0] - wall->startMove[0]) / (wall->moveFactor);
        wall->velocity[1] = (wall->endMove[1] - wall->startMove[1]) / (wall->moveFactor);
        wall->velocity[2] = (wall->endMove[2] - wall->startMove[2]) / (wall->moveFactor);
    } else {
        wall->velocity[0] = 0;
        wall->velocity[1] = 0;
        wall->velocity[2] = 0;
    }

    wall->position[0] += wall->velocity[0] * game->delta;
    wall->position[1] += wall->velocity[1] * game->delta;
    wall->position[2] += wall->velocity[2] * game->delta;

    if (wall->time > 1 + wall->stayTime) {
        vec3 vec = {wall->startMove[0], wall->startMove[1], wall->startMove[2]};
        wall->startMove[0] = wall->endMove[0];
        wall->startMove[1] = wall->endMove[1];
        wall->startMove[2] = wall->endMove[2];
        wall->endMove[0] = vec[0];
        wall->endMove[1] = vec[1];
        wall->endMove[2] = vec[2];
        wall->time = 0;
    }
}

//******************************** Helpers ********************************//
static void cameraControls(GameState *game) {
    trs_Camera *camera = trs_GetCamera();
    static float cameraLookAngle = GLM_PI / 4;

    // Move the camera around the player
    const float lookSpeed = 2;
    if (game->keyboard[SDL_SCANCODE_X] && !game->keyboardPrevious[SDL_SCANCODE_X]) {
        cameraLookAngle += GLM_PI / 2;//game->delta * lookSpeed;
    }
    if (game->keyboard[SDL_SCANCODE_C] && !game->keyboardPrevious[SDL_SCANCODE_C]) {
        cameraLookAngle -= GLM_PI / 2;//game->delta * lookSpeed;
    }

    // Follow the player
    const float distance = 12;
    const float zdistance = 8;
    float cameraX = game->player.x - (distance * cos(cameraLookAngle));
    float cameraY = game->player.y - (distance * sin(cameraLookAngle));
    camera->eyes[0] += ((cameraX) - camera->eyes[0]) * 4 * game->delta;
    camera->eyes[1] += ((cameraY) - camera->eyes[1]) * 4 * game->delta;
    camera->eyes[2] += ((game->player.z + zdistance) - camera->eyes[2]) * 4 * game->delta;
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));
}

bool touchingWall(GameState *game, Level *level, trs_Hitbox hitbox, float x, float y, float z) {
    WallIterator iter;
    Wall *wall = getWallsStart(game, &game->level, &iter);
    while (wall != NULL) {
        if (wall->active) {
            if (trs_Collision(hitbox, x, y, z, wall->hitbox, wall->position[0], wall->position[1], wall->position[2])) {
                level->mostRecentWall = wall;
                return true;
            }
        }
        wall = getWallsNext(&game->level, &iter);
    }

    return false;
}

//******************************** Level loading ********************************//

static float parseFloat(cJSON *num, float def) {
    if (num != NULL && cJSON_IsNumber(num))
        return cJSON_GetNumberValue(num);
    return def;
}

static void parseCoords(cJSON *list, vec3 out) {
    if (list != NULL && cJSON_IsArray(list) && cJSON_GetArraySize(list) == 3) {
        out[0] = parseFloat(cJSON_GetArrayItem(list, 0), 0);
        out[1] = parseFloat(cJSON_GetArrayItem(list, 1), 0);
        out[2] = parseFloat(cJSON_GetArrayItem(list, 2), 0);
    }
}

static bool parseWall(GameState *game, cJSON *wallJSON, Wall *wall) {
    cJSON *type = cJSON_GetObjectItem(wallJSON, "type");
    cJSON *position = cJSON_GetObjectItem(wallJSON, "position");
    cJSON *finalPosition = cJSON_GetObjectItem(wallJSON, "final_position");
    cJSON *move = cJSON_GetObjectItem(wallJSON, "move");
    cJSON *stop = cJSON_GetObjectItem(wallJSON, "stop");
    if (type != NULL && cJSON_IsString(type)) {
        parseCoords(position, wall->position);
        parseCoords(finalPosition, wall->endMove);
        wall->stayTime = parseFloat(stop, 0);
        wall->moveFactor = parseFloat(move, 0);

        if (strcmp(cJSON_GetStringValue(type), "2x2") == 0) {
            wall->model = game->platformModel;
            return true;
        }
    }
    return false;
}

// Loads a level from a csv
bool loadLevel(GameState *game, Level *level, const char *filename) {
    // Parse json
    int size;
    uint8_t *fileString = trs_LoadFile(filename, &size);
    fileString = realloc(fileString, size + 1);
    fileString[size] = 0;
    cJSON *json = cJSON_Parse(fileString);

    if (cJSON_HasObjectItem(json, "walls")) {
        // Parse level
        cJSON *wallList = cJSON_GetObjectItem(json, "walls");
        int size = cJSON_GetArraySize(wallList);
        
        // Parse individual wall
        for (int i = 0; i < size; i++) {
            Wall wall = {0};
            if (parseWall(game, cJSON_GetArrayItem(wallList, i), &wall))
                addWall(level, &wall);
        }
    } else {
        return false;
    }

    cJSON_Delete(json);
    free(fileString);
    return true;
}

//******************************** Level ********************************//
void levelCreate(GameState *game) {
    // Setup camera
    trs_Camera *camera = trs_GetCamera();
    camera->eyes[0] = (game->player.x - 8);
    camera->eyes[1] = (game->player.y - 8);
    camera->eyes[2] = (game->player.z + 1000);
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));

    // Various
    game->level.checkpointID = 0;
    game->level.startTime = game->time;

    loadLevel(game, &game->level, "res/map.json");
    addCheckpoint(&game->level, &((Checkpoint){.position = {5, 0, 0}}));
}

void levelDestroy(GameState *game) {
    for (int i = 0; i < game->level.chunkCount; i++) {
        free(game->level.chunks[i].checkpoints);
        free(game->level.chunks[i].walls);
    }
    free(game->level.chunks);
    game->level.chunkCount = 0;
    game->level.chunks = NULL;
    game->level.mostRecentWall = NULL;
}

bool levelUpdate(GameState *game) {
    cameraControls(game);
    playerUpdate(game, &game->player);

    // Update/draw walls in relavent chunks
    WallIterator iter;
    Wall *wall = getWallsStart(game, &game->level, &iter);
    while (wall != NULL) {
        if (wall->active) {
            updateWall(game, &game->level, wall);
            trs_DrawModelExt(wall->model, wall->position[0], wall->position[1], wall->position[2], 1, 1, 1, 0, 0, 0);
        }
        wall = getWallsNext(&game->level, &iter);
    }

    // Checkpoints
    CheckpointIterator checkIter;
    Checkpoint *checkpoint = getCheckpointsStart(game, &game->level, &checkIter);
    while (checkpoint != NULL) {
        if (checkpoint->active) {
            updateCheckpoint(game, &game->level, checkpoint);
            trs_DrawModelExt(game->flagModel, checkpoint->position[0], checkpoint->position[1], checkpoint->position[2], 1, 1, 1, 0, 0, 0);
        }
        checkpoint = getCheckpointNext(&game->level, &checkIter);
    }

    return true;
}

void levelDraw(GameState *game) {
    // Draw some ground
    const float z = -1;
    trs_DrawModelExt(game->groundPlane, -4, -4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, -4, 0, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, -4, 4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 0, -4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 0, 0, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 0, 4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 4, -4, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 4, 0, z, 1, 1, 1, 0, 0, 0);
    trs_DrawModelExt(game->groundPlane, 4, 4, z, 1, 1, 1, 0, 0, 0);
    
    playerDraw(game, &game->player);
}

void levelDrawUI(GameState *game) {
    trs_Camera *camera = trs_GetCamera();
    char buffer[100];
    const float time = game->time - game->level.startTime;
    snprintf(buffer, 100, "=%02d:%02d:%03d", (int)(time / 60), (int)(fmodf(time, 60)), (int)(fmodf(time * 1000, 1000)));
    const float len = strlen(buffer);
    trs_DrawFont(game->font, 256 - (len * 7), 224 - 9, "%s", buffer);
    trs_DrawFont(game->font, 1, 8 * 2, "Player: %0.0f,%0.0f,%0.0f", game->player.x, game->player.y, game->player.z);

    // Message buffer if there is one active
    if (game->level.messageTime > 0) {

        // Calculate the message fade in/out
        float percent = 1;
        if (game->level.messageTime < 1) {
            percent = game->level.messageTime;
        } else if (game->level.messageTime > MESSAGE_TIME - 1) {
            percent = 1 - (game->level.messageTime - (MESSAGE_TIME - 1));
        }
        // TODO: SLERP PERCENT

        const int len = strlen(game->level.messageBuffer);
        const float xPos = (256 / 2) - (len * 7 * 0.5);
        const float yPos = -8 + (50 * percent);
        trs_DrawFont(game->font, xPos, yPos, "%s", game->level.messageBuffer);
        game->level.messageTime -= game->delta;
        
    }
    
    // Hint
    SDL_RenderCopy(game->renderer, game->hintTex, NULL, &((SDL_Rect){.x = 0, .y = 205, .w = 81, .h = 19}));

    // Draw orientation
    const float startX = 230 + 13;
    const float startY = 13;
    SDL_RenderCopy(game->renderer, game->compassTex, NULL, &((SDL_Rect){.x = startX - 13, .y = startY - 13, .w = 25, .h = 25}));

    // Horizontal orientation
    const float rotation = camera->rotation - ((3 * GLM_PI) / 4);
    const float horiX = (cosf(rotation) * 10);
    const float horiY = (sinf(rotation) * 10);
    SDL_SetRenderDrawColor(game->renderer, 255, 0, 0, 255);
    SDL_RenderDrawLine(game->renderer, startX, startY, startX + horiX, startY + horiY);

    // Vertical orientation
    const float verticalPercent = (camera->rotationZ / GLM_PI);
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 255, 255);
    SDL_RenderDrawLine(game->renderer, startX, startY, startX, startY - (verticalPercent * 20));
    SDL_SetRenderDrawColor(game->renderer, 0, 0, 0, 255);
    SDL_RenderDrawPoint(game->renderer, startX, startY);
}

void levelDisplayMessage(GameState *game, const char *message, ...) {
    va_list l;
    va_start(l, message);
    vsnprintf(game->level.messageBuffer, MESSAGE_BUFFER_SIZE - 1, message, l);
    va_end(l);
    game->level.messageTime = MESSAGE_TIME;
}