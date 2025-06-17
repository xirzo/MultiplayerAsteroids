#ifndef STATE_H
#define STATE_H

#include <stddef.h>
#include "client.h"

typedef struct client_state {
    int is_running;
    int is_connected;
    int is_game_running;
    client_t client;
} client_state_t;

typedef struct server_state {
} server_state_t;
#endif  // !STATE_H
