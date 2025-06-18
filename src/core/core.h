#ifndef CORE_H
#define CORE_H

#include "state.h"

void client_state_init(client_state_t *state);
void process_input_and_move_player(client_state_t *state);

int connect_to_server(client_state_t *state);
void receive_server_message(client_state_t *state);
void send_server_message(client_state_t *state);

#endif  // !CORE_H
