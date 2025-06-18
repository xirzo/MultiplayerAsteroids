#include "core.h"
#include <raylib.h>

void client_state_init(client_state_t *state) {
    state->is_connected = 0;
    state->is_game_running = 0;
    state->player_id = -1;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        client_player_t *player = &state->players[i];

        player->is_active = 0;
        player->size = (vec2){ 80, 80 };
        player->color = (color_t
        ){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        player->velocity = VEC_2_ZERO;
        player->speed = PLAYER_SPEED;
    }

    SetTraceLogLevel(LOG_NONE);
}

void process_input_and_move_player(client_state_t *state) {
    client_player_t *player = &state->players[state->player_id];

    player->velocity = VEC_2_ZERO;

    if (IsKeyDown(KEY_A)) {
        player->velocity.x = -1;
    }
    if (IsKeyDown(KEY_D)) {
        player->velocity.x = 1;
    }
    if (IsKeyDown(KEY_W)) {
        player->velocity.y = -1;
    }
    if (IsKeyDown(KEY_S)) {
        player->velocity.y = 1;
    }

    vec2_normalize(&player->velocity);
    vec2_multiply_scalar(&player->velocity, player->speed * GetFrameTime());
    vec2_add(&player->pos, &player->velocity, &player->pos);
}
