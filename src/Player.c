#include <SDL2/SDL.h>
#include "Software3D.h"
#include "Player.h"
#include "Level.h"

void playerCreate(GameState *game, Player *player) {
    player->x = 0;
    player->y = 0;
    player->z = 0;
    player->onGroundLastFrame = true;
    player->hitbox = trs_GetModelHitbox(game->playerModel);
}

void playerDestroy(GameState *game, Player *player) {
    trs_FreeHitbox(player->hitbox);
}

bool playerOnGround(GameState *game, Player *player) {
    return player->z <= 0 || touchingWall(game, &game->level, player->hitbox, player->x, player->y, player->z - 0.1);
}

void playerUpdate(GameState *game, Player *player) {
    trs_Camera *cam = trs_GetCamera();
    const float speed = 0.3 * game->delta;
    const float gravity = 9.8 * 9.8 * game->delta;
    const float terminalVelocity = -30;
    const float topSpeed = 8;
    const float friction = 0.8 * game->delta;
    const float jumpSpeed = 35;
    const float jumpDuration = 0.3; // in seconds
    const float squishDuration = 0.3;
    game->level.mostRecentWall = NULL;
    const bool onGround = playerOnGround(game, player);
    const Wall *wallBelow = game->level.mostRecentWall;

    // Get player input
    player->velocityX = player->velocityY = 0;
    player->velocityX += (float)game->keyboard[SDL_SCANCODE_RIGHT] - (float)game->keyboard[SDL_SCANCODE_LEFT];
    player->velocityY += (float)game->keyboard[SDL_SCANCODE_UP] - (float)game->keyboard[SDL_SCANCODE_DOWN];
    const bool jump = onGround && game->keyboard[SDL_SCANCODE_Z] && !game->keyboardPrevious[SDL_SCANCODE_Z];

    // Move relative to the camera's placement
    if (player->velocityX != 0 || player->velocityY != 0) {
        player->direction = cam->rotation + atan2f(player->velocityX, player->velocityY);
        player->speed = clamp(player->speed + speed, 0, topSpeed * game->delta);
    } else {
        player->speed = clamp(player->speed - friction, 0, 999);
    }
    float xComponent = cos(player->direction) * player->speed;
    float yComponent = sin(player->direction) * player->speed;

    // Move with the wall the player is standing on
    if (onGround && wallBelow != NULL) {
        xComponent += wallBelow->velocity[0] * game->delta;
        yComponent += wallBelow->velocity[1] * game->delta;
        player->z = wallBelow->position[2] + (wallBelow->hitbox->box[1][2]) + 0.05;
    }

    // Add velocity to position assuming there is no wall in the way
    if (!touchingWall(game, &game->level, player->hitbox, player->x + (xComponent), player->y, player->z))
        player->x += xComponent;
    if (!touchingWall(game, &game->level, player->hitbox, player->x, player->y + (yComponent), player->z))
        player->y += yComponent;


    // Squash animation
    if (onGround && !player->onGroundLastFrame && player->velocityZ < -8) {
        player->squishTimer = squishDuration;
    }

    if (onGround)
        player->velocityZ = 0;

    // Jump & gravity
    if (jump) {
        player->jumpTimer = jumpDuration;
        player->velocityZ = jumpSpeed;
    }
    if (player->jumpTimer > 0) {
        player->jumpTimer -= game->delta;
        const float x = (player->jumpTimer / jumpDuration);
        player->zscale = 1 + ((-powf((2 * x) - 1, 2) + 1) * 0.3);
    } else {
        player->zscale = 1;
    }
    if (!onGround)
        player->velocityZ = clamp(player->velocityZ - gravity, terminalVelocity, 9999999);

    // Collide with walls below
    if (!touchingWall(game, &game->level, player->hitbox, player->x, player->y, player->z + (player->velocityZ * game->delta))) {
        player->z = clamp(player->z + (player->velocityZ * game->delta), 0, 999);
    } else {
        // Sit neatly on the wall below
        if (game->level.mostRecentWall->position[2] + game->level.mostRecentWall->hitbox->box[1][2] < player->z)
            player->z = game->level.mostRecentWall->position[2] + game->level.mostRecentWall->hitbox->box[1][2] + 0.1;
    }

    // Squash animation 2
    if (player->squishTimer > 0) {
        const float percent = (player->squishTimer / squishDuration);
        player->squishTimer -= game->delta;
        player->zscale = 1 - ((-powf((2 * percent) - 1, 2) + 1) * 0.6);
    }

    player->onGroundLastFrame = onGround;

    // Move away from a wall thats moving into the player
    if (touchingWall(game, &game->level, player->hitbox, player->x, player->y, player->z)) {
        player->x += game->level.mostRecentWall->velocity[0] * game->delta;
        player->y += game->level.mostRecentWall->velocity[1] * game->delta;
        player->z += game->level.mostRecentWall->velocity[2] * game->delta;
    }
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

    trs_DrawModelExt(game->playerModel, player->x, player->y, player->z, 1 / player->zscale, 1 / player->zscale, player->zscale, 0, 0, player->drawDirection + (GLM_PI / 2));
}