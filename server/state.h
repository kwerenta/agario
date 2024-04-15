#ifndef STATE_H
#define STATE_H

#include <pthread.h>

#include "../shared/config.h"
#include "../shared/types.h"

typedef struct {
  u32 color;
  Position position;
  u32 score;

  i32 socket;
  pthread_t thread;
} Player;

typedef struct {
  Position position;
} Ball;

typedef struct {
  u8 player_count;
  u16 balls_count;

  Player players[MAX_PLAYERS];
  Ball balls[MAX_BALLS];
} State;

#endif
