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
    const float distance = game->keyboard[SDL_SCANCODE_V] ? 6 : 12;
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
        if (trs_Collision(hitbox, x, y, z, wall->hitbox, wall->x, wall->y, wall->z)) {
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
    if (level->walls[spot].hitbox == NULL)
        level->walls[spot].hitbox = trs_GetModelHitbox(level->walls[spot].model);
}

//******************************** Level ********************************//
void levelCreate(GameState *game) {
    // Setup camera
    trs_Camera *camera = trs_GetCamera();
    camera->eyes[0] = (game->player.x - 8);
    camera->eyes[1] = (game->player.y - 8);
    camera->eyes[2] = (game->player.z + 5000);
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));

    // Setup walls
    game->level.walls = NULL;
    game->level.wallCount = 0;
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .x = 3,
        .y = 0,
        .z = 0
    }));
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .x = 0,
        .y = 3,
        .z = 2
    }));
    addWall(&game->level, &((Wall){
        .model = game->platformModel,
        .x = -3,
        .y = 0,
        .z = 4
    }));
}

void levelDestroy(GameState *game) {

}

void levelUpdate(GameState *game) {
    cameraControls(game);
    playerUpdate(game, &game->player);
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
        trs_DrawModelExt(game->level.walls[i].model, game->level.walls[i].x, game->level.walls[i].y, game->level.walls[i].z, 1, 1, 1, 0, 0, 0);
    }
    
    playerDraw(game, &game->player);
}

void levelDrawUI(GameState *game) {
    
}
