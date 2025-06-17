#ifndef STATE_H
#define STATE_H

#include <stddef.h>
#include "client.h"

typedef struct client_state {
    int is_running;
    int is_connected;
    client_t client;
} client_state_t;

typedef struct server_state {
    size_t players_count;
} server_state_t;
#endif  // !STATE_H
