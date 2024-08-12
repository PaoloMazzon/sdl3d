#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "Software3D.h"
#pragma once

#define MENU_FADE_TIME 1

typedef enum {
    GAME_ROOM_MENU = 0,
    GAME_ROOM_GAME = 1,
} GameRoom;

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
    bool active;
    vec3 position;
    vec3 startMove;
    vec3 endMove;
    trs_Hitbox hitbox;
    trs_Model model;
    
    // For moving platforms
    vec3 velocity; // as x,y,z components
    float time;
    float moveFactor; // time is multiplied by this
    float stayTime; // time this wall stays at both ends
} Wall;

typedef struct Checkpoint_t {
    vec3 position;
} Checkpoint;

typedef struct Chunk_t {
    Wall *walls;
    int wallCount;
    Checkpoint *checkpoints;
    int checkpointCount;
} Chunk;

typedef struct Level_t {
    Wall *walls; // array of walls in the game world
    int wallCount;
    Wall *mostRecentWall; // whatever wall was collided with most recently
    double startTime;
} Level;

typedef struct Menu_t {
    int cursor;
    float timer;
    bool nextRoom;
} Menu;

typedef struct GameState_t {
    // Engine
    SDL_Renderer *renderer;
    bool *keyboard;
    bool *keyboardPrevious;
    double delta;
    double time;
    double fps;

    // Game stuff
    Player player;
    Level level;
    Menu menu;
    GameRoom state;

    // Assets
    trs_Model testModel;
    SDL_Texture *hintTex;
    trs_Font font;
    trs_Font menuFont;
    SDL_Texture *compassTex;
    trs_Model playerModel;
    trs_Model groundPlane;
    trs_Model platformModel;
    trs_Model islandModel;
} GameState;