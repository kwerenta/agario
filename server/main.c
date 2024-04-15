#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include "../shared/config.h"
#include "../shared/types.h"
#include "network.h"
#include "state.h"

int main() {
  const int server_fd = setup_server();
  if (server_fd < 0)
    return 1;

  State state = {.player_count = 0, .balls_count = 0, .players = {0}, .balls = {0}};
  u8 has_started = 0;

  srand(time(NULL));

  pthread_t game_thread;

  for (;;) {
    accept_player(server_fd, state.players);
    state.player_count++;

    if (has_started == 0) {
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
    pthread_join(state.players[i].thread, NULL);
    close(state.players[i].socket);
  }

  close(server_fd);
}
