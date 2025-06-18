#ifndef RENDER_H
#define RENDER_H

#include "state.h"

void init_window();
void render_frame(client_state_t *state);
void render_active_players(client_state_t *state);

#endif  // !RENDER_H
