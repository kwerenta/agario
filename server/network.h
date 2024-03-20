#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

#include "../shared/types.h"

typedef struct Client {
  int client_fd;
  const Position *position;
} Client;

// Creates socket, binds it to the port and starts listening
const int setup_server();

void accept_player(const int server_fd, pthread_t *client_thread, Client *client, Position *state);

void *handle_connection(void *p_client);
void *handle_game_update(void *p_state);

#endif
