#include <stdio.h>
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

  for (int i = 0; i < MAX_PLAYERS; i++) {
    accept_player(server_fd, state.players + i, i);
    state.player_count++;
  }

  printf("All players connected, starting game...\n");

  pthread_t position_thread;
  pthread_create(&position_thread, NULL, handle_game_update, &state);

  // Wait for game to end
  pthread_join(position_thread, NULL);
  printf("Game has ended\n");

  // Wait for all players to disconnect
  for (int i = 0; i < MAX_PLAYERS; i++) {
    pthread_join(state.players[i].thread, NULL);
    close(state.players[i].socket);
  }

  close(server_fd);
}
