#ifndef CONNECTION_H
#define CONNECTION_H

#include "../shared/config.h"
#include "../shared/types.h"
#include "game.h"

#define BUFFER_SIZE 1024

typedef struct State {
  const int fd;
  int is_connected;
  GameState game;
} State;

// Creates socket, assigns address and connects to server
const int setup_connection();

void *handle_connection(void *p_state);
#endif