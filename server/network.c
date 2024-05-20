#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../shared/action_queue.h"
#include "../shared/config.h"
#include "../shared/serialization.h"

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

void accept_player(const int server_fd, State *state, pthread_mutex_t *player_count_mutex, ActionNode **action_queue) {
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

  *player = (Player){.color = 0,
                     .position = {.x = client_id % 2 == 0 ? 0 : 200, .y = client_id % 2 == 0 ? 0 : 200},
                     .score = 0,
                     .has_joined = 0,
                     .socket = client_fd,
                     .thread = 0,
                     .last_message_id = 0};

  if (pthread_create(&player->thread, NULL, player_data_receiver,
                     &(ReceiverParams){
                         .player_count_mutex = player_count_mutex,
                         .action_queue_mutex = state->action_queue_mutex,
                         .action_queue = action_queue,
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

  u8 action;
  u16 message_id;

  while (recv(params.player->socket, buffer, sizeof(buffer), 0) > 0) {
    deserialize_header(buffer, &action, &message_id);

    // JOIN
    if (action == 0) {
      printf("Received JOIN message from client (id=%d)\n", params.player_id);
      deserialize_u32(params.player->color, buffer + 2);
      params.player->has_joined = 1;
      continue;
    }

    // MOVE
    if (action == 1) {
      Position new_position = {0};

      deserialize_f32(new_position.x, buffer + 2);
      deserialize_f32(new_position.y, buffer + 6);

      pthread_mutex_lock(params.action_queue_mutex);

      // Add move sent by client to action queue
      enqueue(params.action_queue,
              (ActionValue){.player_id = params.player_id, .message_id = message_id, .position = new_position});
      params.player->last_message_id = message_id;

      pthread_mutex_unlock(params.action_queue_mutex);
    }

    // SPEED
    if (action == 2) {
      params.player->score += 1;
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

    pthread_mutex_lock(state->action_queue_mutex);

    // Validates all moves send to the server since last tick
    while (*state->action_queue != NULL) {
      ActionValue action = dequeue(state->action_queue);

      float dx = action.position.x - state->players[action.player_id].position.x;
      float dy = action.position.y - state->players[action.player_id].position.y;
      float distance = sqrt(dx * dx + dy * dy);

      // Checks if move was possible
      if (distance <= 1.0 / TICKS_PER_SECOND * (1 + 1.0 / state->players[action.player_id].score + 1)) {
        state->players[action.player_id].position = action.position;
      } else {
        printf("INCORRECT MOVE: Player %d, message %d (x=%f, y=%f)\n", action.player_id, action.message_id,
               action.position.x, action.position.y);
      }
    }

    u32 message_size = serialize_message(buffer, state);
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (state->players[i].socket == 0)
        continue;

      // Id of client that the message is send to
      buffer[5] = i;
      serialize_header(buffer, 0, state->players[i].last_message_id);
      send(state->players[i].socket, buffer, message_size, 0);
    }

    pthread_mutex_unlock(state->action_queue_mutex);

    // Calculates how much time it needs to sleep to complete a full tick
    tick_time = (clock() - start) / (f64)CLOCKS_PER_SEC;
    time_diff = 1.0 / TICKS_PER_SECOND - tick_time;

    if (time_diff > 0) {
      ts.tv_sec = 0;
      ts.tv_nsec = time_diff * 1000 * 1000 * 1000;
      nanosleep(&ts, &ts);
    }
  }

  return NULL;
}

u32 serialize_message(u8 *buffer_start, State *state) {
  u8 *buffer = buffer_start;
  serialize_header(buffer, 0, 0);

  buffer[2] = state->player_count;
  serialize_u16(buffer + 3, state->balls_count);

  buffer += 6;

  for (u8 i = 0; i < MAX_PLAYERS; i++) {
    if (state->players[i].socket == 0)
      continue;

    *buffer = i;

    serialize_u32(buffer + 1, state->players[i].color);
    serialize_f32(buffer + 5, state->players[i].position.x);
    serialize_f32(buffer + 9, state->players[i].position.y);
    serialize_u32(buffer + 13, state->players[i].score);

    buffer += 17;
  }

  for (u16 i = 0; i < MAX_BALLS; i++) {
    if (state->balls[i].position.x == 0 && state->balls[i].position.y == 0)
      continue;

    serialize_f32(buffer, state->balls[i].position.x);
    serialize_f32(buffer + 4, state->balls[i].position.y);
    buffer += 8;
  }

  return buffer - buffer_start;
}
