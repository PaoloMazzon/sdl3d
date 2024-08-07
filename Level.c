#include "Level.h"
#include "Player.h"

//******************************** Helpers ********************************//
static void cameraControls(GameState *game) {
    trs_Camera *camera = trs_GetCamera();
    static float cameraLookAngle = GLM_PI / 4;

    // Move the camera around the player
    const float lookSpeed = 2;
    if (game->keyboard[SDL_SCANCODE_A] && !game->keyboardPrevious[SDL_SCANCODE_A]) {
        cameraLookAngle += GLM_PI / 2;//game->delta * lookSpeed;
    }
    if (game->keyboard[SDL_SCANCODE_D] && !game->keyboardPrevious[SDL_SCANCODE_D]) {
        cameraLookAngle -= GLM_PI / 2;//game->delta * lookSpeed;
    }

    // Follow the player
    float cameraX = game->player.x - (12 * cos(cameraLookAngle));
    float cameraY = game->player.y - (12 * sin(cameraLookAngle));
    camera->eyes[0] += ((cameraX) - camera->eyes[0]) * 4 * game->delta;
    camera->eyes[1] += ((cameraY) - camera->eyes[1]) * 4 * game->delta;
    camera->eyes[2] += ((game->player.z + 8) - camera->eyes[2]) * 4 * game->delta;
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));
}

void levelCreate(GameState *game) {
    // Setup camera
    trs_Camera *camera = trs_GetCamera();
    camera->eyes[0] = (game->player.x - 8);
    camera->eyes[1] = (game->player.y - 8);
    camera->eyes[2] = (game->player.z + 8);
    camera->rotation = atan2f(camera->eyes[1] - game->player.y, camera->eyes[0] - game->player.x) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - game->player.z, sqrtf(powf(camera->eyes[1] - game->player.y, 2) + pow(camera->eyes[0] - game->player.x, 2)));
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

    trs_DrawModelExt(game->cubeModel, 2, 2, 0, 1, 1, 1, 0, 0, 0);
    
    playerDraw(game, &game->player);
}

void levelDrawUI(GameState *game) {

}
