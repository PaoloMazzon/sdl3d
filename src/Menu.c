#include "Menu.h"

void menuStart(GameState *game, Menu *menu) {
    trs_Camera *camera = trs_GetCamera();
    const float lookAtX = 0;
    const float lookAtY = 0;
    const float lookAtZ = 0;
    const float distance = 8;
    const float zdistance = 6;
    camera->rotation = atan2f(camera->eyes[1] - lookAtY, camera->eyes[0] - lookAtX) + GLM_PI;
    camera->rotationZ = -atan2f(camera->eyes[2] - lookAtZ, sqrtf(powf(camera->eyes[1] - lookAtY, 2) + pow(camera->eyes[0] - lookAtX, 2)));
    menu->nextRoom = false;
}

bool menuUpdate(GameState *game, Menu *menu) {
    // DEBUG, SO I DONT HAVE TO SEE THE MENU
    return false;
    if (game->keyboard[SDL_SCANCODE_Z] && !menu->nextRoom) {
        menu->nextRoom = true;
        menu->timer = MENU_FADE_TIME;
    }

    menu->timer -= game->delta;

    if (menu->nextRoom && menu->timer <= 0) {
        return false;
    }
    return true;
}

static float slerp(float percent, float min, float max) {
    const float newPercent = (0.5 * sinf(GLM_PI * (percent - 0.5))) + 0.5;
    return min + (newPercent * (max - min));
}

void menuDraw(GameState *game, Menu *menu) {
    // Camera
    trs_Camera *camera = trs_GetCamera();
    const float lookAtX = 0;
    const float lookAtY = 0;
    const float lookAtZ = 0;
    const float cameraLookAngle = GLM_PI / 4;
    float distance = 8;
    float zdistance = 6;
    if (menu->nextRoom) {
        distance = slerp(1 - (menu->timer / MENU_FADE_TIME), 8, 100);
        zdistance = slerp(1 - (menu->timer / MENU_FADE_TIME), 6, 100);
    }
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
    if (menu->nextRoom)
        textDrawX = slerp((1 - (menu->timer / MENU_FADE_TIME)), (256 * 0.5) - (9 * 8), 280);
    trs_DrawFont(game->menuFont, textDrawX, textDrawY, "Z to play");
}
