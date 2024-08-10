#include "Menu.h"

void menuStart(GameState *game, Menu *menu) {

}

bool menuUpdate(GameState *game, Menu *menu) {
    if (game->keyboard[SDL_SCANCODE_Z])
        return false;
    return true;
}

void menuDraw(GameState *game, Menu *menu) {
    // Camera
    trs_Camera *camera = trs_GetCamera();
    const float lookAtX = 0;
    const float lookAtY = 0;
    const float lookAtZ = 0;
    const float cameraLookAngle = GLM_PI / 4;
    const float distance = 8;
    const float zdistance = 6;
    float cameraX = lookAtX - (distance * cos(cameraLookAngle));
    float cameraY = lookAtY - (distance * sin(cameraLookAngle));
    camera->eyes[0] += ((cameraX) - camera->eyes[0]) * 4 * game->delta;
    camera->eyes[1] += ((cameraY) - camera->eyes[1]) * 4 * game->delta;
    camera->eyes[2] += ((lookAtZ + zdistance) - camera->eyes[2]) * 4 * game->delta;
    camera->rotation = atan2f(camera->eyes[1] - lookAtY, camera->eyes[0] - lookAtX) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - lookAtZ, sqrtf(powf(camera->eyes[1] - lookAtY, 2) + pow(camera->eyes[0] - lookAtX, 2)));

    // Rotate model
    trs_DrawModelExt(game->islandModel, 0, 0, -1.5, 1, 1, 1, 0, 0, game->time);
}

void menuDrawUI(GameState *game, Menu *menu) {
    // Draw play button
    float textDrawX = (256 * 0.5) - (9 * 8);
    float textDrawY = (224 * 0.5) - 8;
    trs_DrawFont(game->menuFont, textDrawX, textDrawY, "Z to play");
}
