#include <logger.h>
#include "render.h"
#include "core.h"

int main(void) {
    client_state_t state = { 0 };
    client_state_init(&state);

    if ((connect_to_server(&state)) != 0) {
        return 1;
    }

    init_window();

    while (!should_window_close()) {
        receive_server_message(&state);

        if (state.is_game_running) {
            process_input_and_move_player(&state);
            send_server_message(&state);
        }

        render_frame(&state);
    }

    close_window();
    return 0;
}
