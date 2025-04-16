#ifndef MAIN_H
#define MAIN_H

#include "gba.h"

#define MAX_ALIENS 12
#define MAX_BULLETS 5
#define MAX_ALIEN_BULLETS 5
#define ALIEN_ROWS 2
#define ALIEN_COLS 4
#define PLAYER_SPEED 2
#define BULLET_SPEED 3
#define ALIEN_BULLET_SPEED 2
#define RUSHER_CHANCE 15
#define PLAYER_LIVES 3
#define BULLET_COOLDOWN 20

typedef struct {
    int x;
    int y;
    int lives;
    int bulletCooldown;
} Player;

typedef enum {
    NORMAL,
    RUSHER
} AlienType;

typedef struct {
    int x;
    int y;
    int dx;
    int dy;
    int alive;
    AlienType type;
    int rushCooldown;
} Alien;

typedef struct {
    int x;
    int y;
    int active;
} Bullet;

typedef enum {
    START,
    PLAY,
    WIN,
    LOSE,
} GameState;
#endif