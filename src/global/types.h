#ifndef TYPES_H
#define TYPES_H

#include "vec2.h"

typedef struct client_player_t {
    vec2 pos;
} client_player_t;

typedef struct server_player {
    vec2 pos;
} server_player_t;

#endif  // !TYPES_H
