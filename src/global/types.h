#ifndef TYPES_H
#define TYPES_H

#include "vec2.h"

typedef struct color {
    unsigned short r;
    unsigned short g;
    unsigned short b;
    unsigned short a;
} color_t;

typedef struct client_player_t {
    vec2 pos;
    vec2 size;
    vec2 velocity;
    float speed;
    color_t color;
    int is_active;
} client_player_t;

typedef struct server_player {
    int client_id;
    int is_active;
    vec2 pos;
} server_player_t;

#endif  // !TYPES_H
