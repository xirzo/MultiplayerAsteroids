#include "client.h"
#include "server.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

int sr_client_connect(client_t *client, const char *server_ip, unsigned short port) {
    if (!server_ip) {
        LOG_ERROR("CLIENT ERROR: ip is NULL");
        return -1;
    }

    client->addr_len = sizeof(client->server_addr);

    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    client->server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (inet_pton(AF_INET, server_ip, &client->server_addr.sin_addr) <= 0) {
        LOG_ERROR("CLIENT ERROR: invalid address or address not supported");
        return -1;
    }

    if ((client->server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        LOG_ERROR("CLIENT ERROR: socket creation failed: %s", strerror(errno));
        return -1;
    }

    if (connect(
            client->server_fd, (struct sockaddr *)&client->server_addr, sizeof(client->server_addr)
        )
        < 0) {
        LOG_ERROR("CLIENT ERROR: connection with the server failed: %s", strerror(errno));
        return -1;
    }

    LOG_INFO("CLIENT: connected to the server");
    return 0;
}

void sr_client_close(client_t *client) {
    close(client->server_fd);
    LOG_INFO("CLIENT: connection closed");
}

int sr_receive_server_message(client_t *client, server_message_t *msg) {
    if (!client || !msg) {
        LOG_ERROR("CLIENT ERROR: invalid arguments");
        return -1;
    }

    static int non_blocking = 0;

    if (!non_blocking) {
        int flags = fcntl(client->server_fd, F_GETFL, 0);
        if (fcntl(client->server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
            LOG_ERROR("CLIENT ERROR: failed to set non-blocking mode: %s", strerror(errno));
            return -1;
        }
        non_blocking = 1;
    }

    ssize_t bytes_read = recv(client->server_fd, msg, sizeof(server_message_t), 0);

    if (bytes_read == sizeof(server_message_t)) {
        return 0;
    } else if (bytes_read == 0) {
        LOG_INFO("CLIENT: server disconnected");
        return -1;
    } else if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -3;
        }
        LOG_ERROR("CLIENT ERROR: receive failed: %s", strerror(errno));
        return -2;
    } else {
        LOG_ERROR("CLIENT ERROR: incomplete message received");
        return -2;
    }
}

int sr_send_message_to_server(client_t *client, const client_message_t *msg) {
    if (!client || !msg) {
        LOG_ERROR("CLIENT ERROR: invalid arguments");
        return -1;
    }

    ssize_t bytes_sent = send(client->server_fd, msg, sizeof(client_message_t), 0);

    if (bytes_sent != sizeof(client_message_t)) {
        LOG_ERROR("CLIENT ERROR: failed to send message: %s", strerror(errno));
        return -2;
    }

    return 0;
}
