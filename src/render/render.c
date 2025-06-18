#include "render.h"
#include <raylib.h>
#include "types.h"

void init_window() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "game");
    SetTargetFPS(TARGET_FPS);
}

void render_frame(client_state_t *state) {
    render_active_players(state);
}

void render_active_players(client_state_t *state) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        client_player_t *player = &state->players[i];

        if (!player->is_active) {
            continue;
        }

        Color color = (Color){ player->color.r, player->color.g, player->color.b, player->color.a };
        DrawRectangle(player->pos.x, player->pos.y, player->size.x, player->size.y, color);
    }
}
