#ifndef NETWORK_H
#define NETWORK_H

#include <pthread.h>

#include "../shared/action_queue.h"
#include "../shared/types.h"

#include "state.h"

#define BUFFER_SIZE 4096

typedef struct {
  pthread_mutex_t *player_count_mutex;
  pthread_mutex_t *action_queue_mutex;
  ActionNode **action_queue;
  u8 *player_count;

  u8 player_id;
  Player *player;
} ReceiverParams;

// Creates socket, binds it to the port and starts listening
const int setup_server();

void accept_player(const int server_fd, State *state, pthread_mutex_t *player_count_mutex, ActionNode **action_queue);

void *player_data_receiver(void *p_client);
void *handle_game_update(void *p_state);

u32 serialize_message(u8 *buffer, State *state);

#endif
