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
  pthread_mutex_t *player_state_mutex;
  u8 player_count;
  u16 balls_count;
  struct timespec last_ball_spawn_time;
  struct timespec last_score_loss_time;

  Player players[MAX_PLAYERS];
  Ball balls[MAX_BALLS];
} State;

// Check if player a is inside player b
u8 check_player_collision(Player *a, Player *b);

void validate_move(Player *player, const ActionValue *action);

void handle_player_collisions(State *state);
void handle_ball_collisions(State *state);
void handle_ball_spawn(State *state);
void handle_score_loss(State *state);
void handle_speed_time(Player *player);

#endif
