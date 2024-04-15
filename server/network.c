#include <arpa/inet.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

void accept_player(const int server_fd, Player *player, i32 index) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  const int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  *player = (Player){.color = index,
                     .position = {.x = index == 0 ? 0 : 200, .y = index == 0 ? 0 : 200},
                     .socket = client_fd,
                     .score = 0};

  if (pthread_create(&player->thread, NULL, player_data_receiver, player)) {
    perror("failed to create thread for client");
  }

  printf("Client connected from %s on port %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
}

void *player_data_receiver(void *p_receiver_params) {
  Player *player = (Player *)p_receiver_params;
  u8 buf[2];

  // TEMP: Fixed message size
  while (recv(player->socket, buf, sizeof(buf), 0) == sizeof(buf)) {
    if (buf[0] == 0 && buf[1] == 5) {
      player->score += 1;
    }
  }

  printf("Stopped handling connection\n");
  return NULL;
}

void *handle_game_update(void *p_state) {
  State *state = (State *)p_state;
  char buf[1024];

  clock_t start;
  struct timespec ts;
  f64 tick_time, time_diff;

  while (1) {
    start = clock();
    for (int i = 0; i < state->player_count; i++) {
      if (i == 0) {
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

    for (int i = 0; i < state->player_count; i++) {
      snprintf(buf, sizeof(buf), "P,%f,%f,%d,P,%f,%f,%d", state->players[0].position.x, state->players[0].position.y,
               state->players[0].score, state->players[1].position.x, state->players[1].position.y,
               state->players[1].score);
      send(state->players[i].socket, buf, strlen(buf) + 1, 0);
    }
  }

  return NULL;
}
