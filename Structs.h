#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "Software3D.h"
#pragma once

typedef struct Player_t {
    float x, y, z;
    float velocityX, velocityY, velocityZ;
    float speed;
    float direction;
    float drawDirection;
    float jumpTimer;
    float zscale; // for animations
    bool onGroundLastFrame;
    float squishTimer; // for animations
    trs_Hitbox hitbox;
} Player;

typedef struct Wall_t {
    float x, y, z;
    vec3 velocity; // as x,y,z components
    trs_Hitbox hitbox;
    bool active;
} Wall;

typedef struct Level_t {
    Wall *walls; // array of walls in the game world
} Level;

typedef struct GameState_t {
    // Engine
    SDL_Renderer *renderer;
    bool *keyboard;
    bool *keyboardPrevious;
    double delta;
    double time;

    // Game stuff
    Player player;
    Level level;

    // Assets
    trs_Model testModel;
    trs_Font font;
    SDL_Texture *compassTex;
    trs_Model playerModel;
    trs_Model groundPlane;
    trs_Model cubeModel;
} GameState;