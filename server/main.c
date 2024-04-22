#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../shared/config.h"
#include "../shared/types.h"

#include "network.h"
#include "state.h"

int main() {
  const int server_fd = setup_server();
  if (server_fd < 0)
    return 1;

  srand(time(NULL));

  State state = {.player_count = 0, .balls_count = START_BALLS, .players = {0}, .balls = {0}};

  for (int i = 0; i < START_BALLS; i++) {
    // TODO: Add check if position is free
    state.balls[i].position.x = (float)rand() / ((float)RAND_MAX / MAP_WIDTH);
    state.balls[i].position.y = (float)rand() / ((float)RAND_MAX / MAP_HEIGHT);
  }

  u8 has_started = 0;

  pthread_t game_thread;
  pthread_mutex_t player_count_mutex;

  if (pthread_mutex_init(&player_count_mutex, NULL) != 0) {
    perror("Failed to init player count mutex");
    return 1;
  }

  for (;;) {
    accept_player(server_fd, &state, &player_count_mutex);

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
}
