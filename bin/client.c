#include <logger.h>
#include <stdio.h>

int main(void) {
    FILE* log_file = fopen("client_log", "w");
    logger_set_output_file(log_file);

    fclose(log_file);
    return 0;
}
