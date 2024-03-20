#include <stdio.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>

#include "../shared/config.h"
#include "../shared/types.h"
#include "network.h"

int main() {
  const int server_fd = setup_server();
  if (server_fd < 0)
    return 1;

  Position position = {.x = 0, .y = 0};

  Client clients[MAX_PLAYERS] = {};
  pthread_t client_threads[MAX_PLAYERS] = {};

  for (int i = 0; i < MAX_PLAYERS; i++) {
    accept_player(server_fd, client_threads + i, clients + i, &position);
  }

  printf("All players connected, starting game...\n");

  pthread_t position_thread;
  pthread_create(&position_thread, NULL, handle_game_update, &position);

  // Wait for game to end
  pthread_join(position_thread, NULL);
  printf("Game has ended\n");

  // Wait for all players to disconnect
  for (int i = 0; i < MAX_PLAYERS; i++) {
    pthread_join(client_threads[i], NULL);
    close(clients[i].client_fd);
  }

  close(server_fd);
}
