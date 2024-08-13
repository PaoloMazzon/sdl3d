#include "Level.h"
#include "Player.h"

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

bool touchingWall(Level *level, trs_Hitbox hitbox, float x, float y, float z) {
    for (int i = 0; i < level->wallCount; i++) {
        if (level->walls[i].active == false)
            continue;
        Wall *wall = &level->walls[i];
        if (trs_Collision(hitbox, x, y, z, wall->hitbox, wall->position[0], wall->position[1], wall->position[2])) {
            level->mostRecentWall = &level->walls[i];
            return true;
        }
    }
    return false;
}

void addWall(Level *level, Wall *wall) {
    // Search for an open wall slot first
    int spot = -1;
    for (int i = 0; i < level->wallCount && spot == -1; i++)
        if (level->walls[i].active == false)
            spot = i;
    
    // Extend the list
    if (spot == -1) {
        level->walls = realloc(level->walls, (level->wallCount + 10) * sizeof(struct Wall_t));
        for (int i = level->wallCount; i < level->wallCount + 10; i++)
            level->walls[i].active = false;
        spot = level->wallCount;
        level->wallCount += 10;
    }

    // Copy the wall
    level->walls[spot] = *wall;
    level->walls[spot].active = true;
    level->walls[spot].startMove[0] = level->walls[spot].position[0];
    level->walls[spot].startMove[1] = level->walls[spot].position[1];
    level->walls[spot].startMove[2] = level->walls[spot].position[2];
    if (level->walls[spot].hitbox == NULL)
        level->walls[spot].hitbox = trs_GetModelHitbox(level->walls[spot].model);
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
    game->level.startTime = game->time;

    // Setup walls
    game->level.walls = NULL;
    game->level.wallCount = 0;
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .position = {3, 0, 0}
    }));
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .position = {0, 3, 2}
    }));
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .position = {-3, 0, 4}
    }));
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .position = {0, -3, 6},
        .endMove = {4, -3, 6},
        .stayTime = 1,
        .moveFactor = 2
    }));
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .position = {6, 6, 0},
        .endMove = {6, 6, 6},
        .stayTime = 1,
        .moveFactor = 2
    }));
}

void levelDestroy(GameState *game) {

}

bool levelUpdate(GameState *game) {
    cameraControls(game);
    playerUpdate(game, &game->player);

    for (int i = 0; i < game->level.wallCount; i++) {
        if (game->level.walls[i].active)
            updateWall(game, &game->level, &game->level.walls[i]);
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

    // Draw walls
    for (int i = 0; i < game->level.wallCount; i++) {
        if (game->level.walls[i].active == false)
            continue;
        trs_DrawModelExt(game->level.walls[i].model, game->level.walls[i].position[0], game->level.walls[i].position[1], game->level.walls[i].position[2], 1, 1, 1, 0, 0, 0);
    }
    
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
