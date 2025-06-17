#ifndef STATE_H
#define STATE_H

#include <stddef.h>
#include "client.h"
#include "settings.h"
#include "types.h"

typedef struct client_state {
    int is_running;
    int is_connected;
    int is_game_running;
    int player_id;
    client_t client;
} client_state_t;

typedef struct server_state {
    int is_game_running;
    server_player_t players[MAX_PLAYERS];
} server_state_t;
#endif  // !STATE_H
