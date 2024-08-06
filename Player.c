#include <SDL2/SDL.h>
#include "Software3D.h"
#include "Player.h"

void playerCreate(GameState *game, Player *player) {
    player->x = 0;
    player->y = 0;
    player->z = 0;
    player->onGroundLastFrame = true;
    player->hitbox = trs_CalcHitbox(game->playerModel);
}

void playerDestroy(GameState *game, Player *player) {
    trs_FreeHitbox(player->hitbox);
}

bool playerOnGround(GameState *game, Player *player) {
    return player->z <= 0;
}

void playerUpdate(GameState *game, Player *player) {
    trs_Camera *cam = trs_GetCamera();
    const float speed = 0.3 * game->delta;
    const float gravity = 15 * game->delta;
    const float terminalVelocity = -10;
    const float topSpeed = 8;
    const float friction = 0.8 * game->delta;
    const float jumpSpeed = 30;
    const float jumpDuration = 0.8; // in seconds
    const float squishDuration = 0.3;

    // Get player input
    player->velocityX = player->velocityY = player->velocityZ = 0;
    player->velocityX = (float)game->keyboard[SDL_SCANCODE_RIGHT] - (float)game->keyboard[SDL_SCANCODE_LEFT];
    player->velocityY = (float)game->keyboard[SDL_SCANCODE_UP] - (float)game->keyboard[SDL_SCANCODE_DOWN];
    const bool jump = playerOnGround(game, player) && game->keyboard[SDL_SCANCODE_Z];

    // Move relative to the camera's placement
    if (player->velocityX != 0 || player->velocityY != 0) {
        player->direction = cam->rotation + atan2f(player->velocityX, player->velocityY);
        player->speed = clamp(player->speed + speed, 0, topSpeed * game->delta);
    } else {
        player->speed = clamp(player->speed - friction, 0, 999);
    }
    const float xComponent = cos(player->direction);
    const float yComponent = sin(player->direction);

    player->x += xComponent * player->speed;
    player->y += yComponent * player->speed;

    // Jump & gravity
    if (jump)
        player->jumpTimer = jumpDuration;
    if (player->jumpTimer > 0) {
        player->jumpTimer = clamp(player->jumpTimer - game->delta, 0, 999);
        player->velocityZ += jumpSpeed * (player->jumpTimer / jumpDuration) * game->delta;
        const float x = (player->jumpTimer / jumpDuration);
        player->zscale = 1 + ((-powf((2 * x) - 1, 2) + 1) * 0.3);
    } else {
        player->zscale = 1;
    }
    player->velocityZ = clamp(player->velocityZ - gravity, terminalVelocity, 9999999);
    player->z = clamp(player->z + (player->velocityZ), 0, 999);

    // Squash animation
    if (playerOnGround(game, player) && !player->onGroundLastFrame) {
        player->squishTimer = squishDuration;
    }
    if (player->squishTimer > 0) {
        const float percent = (player->squishTimer / squishDuration);
        player->squishTimer -= game->delta;
        player->zscale = 1 - ((-powf((2 * percent) - 1, 2) + 1) * 0.8);
    }

    player->onGroundLastFrame = playerOnGround(game, player);
}

static double normalizeAngle(double angle) {
    while (angle < 0) {
        angle += 2 * GLM_PI;
    }
    while (angle >= 2 * GLM_PI) {
        angle -= 2 * GLM_PI;
    }
    return angle;
}

void playerDraw(GameState *game, Player *player) {
    player->drawDirection = normalizeAngle(player->drawDirection);
    player->direction = normalizeAngle(player->direction);
    float difference = (player->direction - player->drawDirection);
    if (difference > GLM_PI) {
        difference -= 2 * GLM_PI;
    } else if (difference < -GLM_PI) {
        difference += 2 * GLM_PI;
    }
    player->drawDirection += difference * 10 * game->delta;
    player->drawDirection = normalizeAngle(player->drawDirection);

    trs_DrawModelExt(game->playerModel, player->x, player->y, player->z, 1, 1, player->zscale, 0, 0, player->drawDirection + (GLM_PI / 2));
}