#ifndef TYPES_H
#define TYPES_H

#include <raylib.h>
#include "vec2.h"

typedef struct client_player_t {
    vec2 pos;
    vec2 size;
    Color color;
    int is_active;
} client_player_t;

typedef struct server_player {
    int client_id;
    vec2 pos;
} server_player_t;

#endif  // !TYPES_H
