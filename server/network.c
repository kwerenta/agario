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

void accept_player(const int server_fd, State *state, pthread_mutex_t *player_count_mutex) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  const int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

  if (state->player_count == MAX_PLAYERS) {
    // TODO: Send error to client
    printf("Player couldn't join. Server is full\n");
    close(client_fd);
    return;
  }

  pthread_mutex_lock(player_count_mutex);

  // Find empty client_id
  u8 client_id = 0;
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (state->players[i].socket == 0) {
      client_id = i;
      break;
    }
  }

  Player *player = &state->players[client_id];

  *player = (Player){
      .color = 0,
      .position = {.x = client_id % 2 == 0 ? 0 : 200, .y = client_id % 2 == 0 ? 0 : 200},
      .score = 0,
      .has_joined = 0,
      .socket = client_fd,
      .thread = 0,
  };

  if (pthread_create(&player->thread, NULL, player_data_receiver,
                     &(ReceiverParams){
                         .player_count_mutex = player_count_mutex,
                         .player_count = &state->player_count,
                         .player_id = client_id,
                         .player = player,
                     })) {
    perror("failed to create thread for client");
  }

  state->player_count++;

  printf("Client %s:%d connected to the server. Player count: %d\n", inet_ntoa(client_addr.sin_addr),
         htons(client_addr.sin_port), state->player_count);

  pthread_mutex_unlock(player_count_mutex);
}

void *player_data_receiver(void *p_receiver_params) {
  ReceiverParams params = *((ReceiverParams *)p_receiver_params);
  u8 buffer[BUFFER_SIZE] = {0};

  while (recv(params.player->socket, buffer, sizeof(buffer), 0) > 0) {
    if (buffer[0] == 0) {
      printf("Received JOIN message from client (id=%d)\n", params.player_id);
      params.player->color = ntohl(*(u32 *)(buffer + 1));
      params.player->has_joined = 1;
      continue;
    }

    if (buffer[0] == 0b10000000) {
      params.player->score += 1;
    }

    if (buffer[0] == 0b01000000) {
      params.player->position.x = *(f32 *)(buffer + 1);
      params.player->position.y = *(f32 *)(buffer + 5);
    }
  }

  pthread_mutex_lock(params.player_count_mutex);

  (*params.player_count)--;
  params.player->socket = 0;
  params.player->thread = 0;

  printf("Client disconencted from server. Player count: %d\n", *params.player_count);
  pthread_mutex_unlock(params.player_count_mutex);

  return NULL;
}

void *handle_game_update(void *p_state) {
  printf("Game has started\n");

  State *state = (State *)p_state;
  u8 buffer[BUFFER_SIZE] = {0};

  clock_t start;
  struct timespec ts;
  f64 tick_time, time_diff;

  for (;;) {
    if (state->player_count == 0) {
      // printf("All players disconnected. Stopping game\n");
      // return NULL;
      sleep(1);
    }

    start = clock();

    // TODO: Game logic, validate player moves

    tick_time = (clock() - start) / (f64)CLOCKS_PER_SEC;
    time_diff = 1.0 / TICKS_PER_SECOND - tick_time;

    if (time_diff > 0) {
      ts.tv_sec = 0;
      ts.tv_nsec = time_diff * 1000 * 1000 * 1000;
      nanosleep(&ts, &ts);
    }

    serialize_message(buffer, state);
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (state->players[i].socket == 0)
        continue;

      // Id of client that the message is send to
      buffer[4] = i;
      send(state->players[i].socket, buffer, state->player_count * 17 + 5, 0);
    }
  }

  return NULL;
}

void serialize_message(u8 *buffer, State *state) {
  // Header
  buffer[0] = 0b01000000;

  buffer[1] = state->player_count;
  *((u16 *)(buffer + 2)) = htons(state->balls_count);

  u32 byte_offset = 0;
  for (u8 i = 0; i < MAX_PLAYERS; i++) {
    if (state->players[i].socket == 0)
      continue;

    buffer[5 + byte_offset] = i;

    *(u32 *)(buffer + 6 + byte_offset) = htonl(state->players[i].color);
    *(f32 *)(buffer + 10 + byte_offset) = state->players[i].position.x;
    *(f32 *)(buffer + 14 + byte_offset) = state->players[i].position.y;
    *(u32 *)(buffer + 18 + byte_offset) = htonl(state->players[i].score);

    byte_offset += 17;
  }
}
