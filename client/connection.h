#ifndef CONNECTION_H
#define CONNECTION_H

#include <pthread.h>

#include "../shared/action_queue.h"
#include "../shared/config.h"
#include "../shared/types.h"

#include "game.h"

#define BUFFER_SIZE 1024

typedef struct State {
  const int fd;
  u8 is_connected;
  u8 is_running;
  GameState game;

  u16 last_message_id;
  ActionNode **action_queue;
  pthread_mutex_t *action_queue_mutex;
} State;

// Creates socket, assigns address and connects to server
const int setup_connection();

void *handle_connection(void *p_state);
#endif
