#ifndef STATE_H
#define STATE_H

#include <pthread.h>

#include "../shared/action_queue.h"
#include "../shared/config.h"
#include "../shared/types.h"

typedef struct {
  u32 color;
  Position position;
  u32 score;
  i32 speed_time;

  u8 has_joined;
  i32 socket;
  pthread_t thread;
  u16 last_message_id;
} Player;

typedef struct {
  Position position;
} Ball;

typedef struct {
  ActionNode **action_queue;
  pthread_mutex_t *action_queue_mutex;
  u8 player_count;
  u16 balls_count;

  Player players[MAX_PLAYERS];
  Ball balls[MAX_BALLS];
} State;

#endif
