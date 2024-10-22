#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../shared/action_queue.h"
#include "../shared/config.h"
#include "../shared/random.h"
#include "../shared/types.h"
#include "../shared/utils.h"

#include "network.h"
#include "state.h"

int main(int argc, char **argv) {
  u32 port = DEFAULT_PORT;

  if (argc == 2) {
    port = get_port_from_string(argv[1]);

    if (port == 0) {
      perror("Invalid port");
      return 1;
    }
  }

  const int server_fd = setup_server(port);
  if (server_fd < 0)
    return 1;

  srand(time(NULL));

  pthread_mutex_t player_count_mutex, action_queue_mutex;

  if (pthread_mutex_init(&player_count_mutex, NULL) != 0) {
    perror("Failed to init player count mutex");
    return 1;
  }

  if (pthread_mutex_init(&action_queue_mutex, NULL) != 0) {
    perror("Failed to init action queue mutex");
    return 1;
  }

  ActionNode *action_queue = NULL;

  State state = {.action_queue = &action_queue,
                 .action_queue_mutex = &action_queue_mutex,
                 .player_state_mutex = &player_count_mutex,
                 .player_count = 0,
                 .balls_count = START_BALLS,
                 .last_ball_spawn_time = {.tv_sec = 0, .tv_nsec = 0},
                 .last_score_loss_time = {.tv_sec = 0, .tv_nsec = 0},
                 .players = {{0}},
                 .balls = {{{0}}}};

  for (int i = 0; i < START_BALLS; i++) {
    // TODO: Add check if position is free
    state.balls[i].position.x = random_range(BALL_SIZE, MAP_WIDTH - BALL_SIZE);
    state.balls[i].position.y = random_range(BALL_SIZE, MAP_HEIGHT - BALL_SIZE);
  }

  u8 has_started = 0;
  pthread_t game_thread;

  for (;;) {
    accept_player(server_fd, &state, &player_count_mutex, &action_queue);

    if (has_started == 0 && state.player_count > 0) {
      if (pthread_create(&game_thread, NULL, handle_game_update, &state)) {
        perror("Failed to start game thread");
        exit(1);
      }
      has_started = 1;
    }
  }

  // Wait for game to end
  pthread_join(game_thread, NULL);
  printf("Game has ended\n");

  // Wait for all players to disconnect
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (state.players[i].thread != 0)
      pthread_join(state.players[i].thread, NULL);

    if (state.players[i].socket != 0)
      close(state.players[i].socket);
  }

  close(server_fd);

  pthread_mutex_destroy(&player_count_mutex);
  pthread_mutex_destroy(&action_queue_mutex);
}
