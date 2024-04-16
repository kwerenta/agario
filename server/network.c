#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../shared/config.h"

#include "network.h"

const int setup_server() {
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(DEFAULT_PORT);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind error:");
    return -1;
  }

  socklen_t addr_len = sizeof(addr);
  getsockname(fd, (struct sockaddr *)&addr, &addr_len);
  printf("Server is listening on port %d\n", (int)ntohs(addr.sin_port));

  if (listen(fd, 1)) {
    perror("listen error:");
    return -1;
  }

  return fd;
}

void accept_player(const int server_fd, Player *players, u8 *player_count, pthread_mutex_t *player_count_mutex) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  const int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

  if (*player_count == MAX_PLAYERS) {
    // TODO: Send error to client
    printf("Player couldn't join. Server is full\n");
    close(client_fd);
    return;
  }

  pthread_mutex_lock(player_count_mutex);

  // Find empty client_id
  u8 client_id = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (players[i].socket == 0) {
      client_id = i;
      break;
    }
  }

  Player *player = &players[client_id];

  u32 color = (1 + rand() / ((RAND_MAX + 1u) / 0xFFFFFF)) << 8;

  *player = (Player){.color = color,
                     .position = {.x = client_id % 2 == 0 ? 0 : 200, .y = client_id % 2 == 0 ? 0 : 200},
                     .socket = client_fd,
                     .score = 0};

  if (pthread_create(&player->thread, NULL, player_data_receiver,
                     &(ReceiverParams){
                         .player_count_mutex = player_count_mutex, .player_count = player_count, .player = player})) {
    perror("failed to create thread for client");
  }

  (*player_count)++;

  printf("Client %s:%d connected to the server. Player count: %d\n", inet_ntoa(client_addr.sin_addr),
         htons(client_addr.sin_port), *player_count);

  pthread_mutex_unlock(player_count_mutex);
}

void *player_data_receiver(void *p_receiver_params) {
  ReceiverParams *params = (ReceiverParams *)p_receiver_params;
  Player *player = params->player;
  u8 buf[2];

  // TEMP: Fixed message size
  while (recv(player->socket, buf, sizeof(buf), 0) == sizeof(buf)) {
    if (buf[0] == 0 && buf[1] == 5) {
      player->score += 1;
    }
  }

  pthread_mutex_lock(params->player_count_mutex);

  player->socket = 0;
  player->thread = 0;
  (*params->player_count)--;

  printf("Client disconencted from server. Player count: %d\n", *params->player_count);
  pthread_mutex_unlock(params->player_count_mutex);

  return NULL;
}

void *handle_game_update(void *p_state) {
  printf("Game has started\n");

  State *state = (State *)p_state;
  u8 buffer[1024] = {0};

  clock_t start;
  struct timespec ts;
  f64 tick_time, time_diff;

  for (;;) {
    if (state->player_count == 0) {
      printf("All players disconnected. Stopping game\n");
      return NULL;
    }

    start = clock();
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (state->players[i].socket == 0)
        continue;

      if (i % 2 == 0) {
        state->players[i].position.x += 0.5;
        state->players[i].position.y += 0.5;
      } else {
        state->players[i].position.x -= 0.5;
        state->players[i].position.y -= 0.5;
      }
    }

    tick_time = (clock() - start) / (f64)CLOCKS_PER_SEC;
    time_diff = 1.0 / TICKS_PER_SECOND - tick_time;

    if (time_diff > 0) {
      ts.tv_sec = 0;
      ts.tv_nsec = time_diff * 1000 * 1000 * 1000;
      nanosleep(&ts, &ts);
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (state->players[i].socket == 0)
        continue;

      serialize_message(buffer, state, i);
      send(state->players[i].socket, buffer, state->player_count * 16 + 4, 0);
    }
  }

  return NULL;
}

void serialize_message(u8 *buffer, State *state, int current_player) {
  // Header
  buffer[0] = 0;

  buffer[1] = state->player_count;
  *((u16 *)(buffer + 2)) = htons(state->balls_count);

  int byte_offset = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (state->players[i].socket == 0)
      continue;

    *((u32 *)(buffer + 4 + byte_offset)) = htonl(state->players[i].color);
    buffer[8 + byte_offset] = state->players[i].position.x;
    buffer[12 + byte_offset] = state->players[i].position.y;
    *((u32 *)(buffer + 16 + byte_offset)) = htonl(state->players[i].score);

    byte_offset += 16;
  }
}
