#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

#include "../shared/types.h"
#include "state.h"

// Creates socket, binds it to the port and starts listening
const int setup_server();

void accept_player(const int server_fd, Player *players);

void *player_data_receiver(void *p_client);
void *handle_game_update(void *p_state);

void serialize_message(u8 *buffer, State *state, int current_player);

#endif
