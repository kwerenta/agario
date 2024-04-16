#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

#include "../shared/types.h"
#include "state.h"

typedef struct {
  pthread_mutex_t *player_count_mutex;
  u8 *player_count;
  Player *player;
} ReceiverParams;

// Creates socket, binds it to the port and starts listening
const int setup_server();

void accept_player(const int server_fd, Player *players, u8 *player_count, pthread_mutex_t *player_count_mutex);

void *player_data_receiver(void *p_client);
void *handle_game_update(void *p_state);

void serialize_message(u8 *buffer, State *state, int current_player);

#endif
