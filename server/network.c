#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
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

void accept_player(const int server_fd, pthread_t *client_thread, Client *client, Position *state) {
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  const int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
  *client = (Client){.client_fd = client_fd, .position = state};

  if (pthread_create(client_thread, NULL, handle_connection, client)) {
    perror("failed to create thread for client");
  }

  printf("Client connected from %s on port %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
}

void *handle_connection(void *p_client) {
  Client *client = (Client *)p_client;

  char buf[1024];

  while (1) {
    snprintf(buf, sizeof(buf), "P0,%d,%d,P1,%d,%d", client->position->x, client->position->y, 15 - client->position->x,
             10 - client->position->y);
    send(client->client_fd, buf, strlen(buf) + 1, 0);
    sleep(1);

    if (client->position->x >= 15)
      break;
  }

  printf("Stopped handling connection\n");
  return NULL;
}

void *handle_game_update(void *p_state) {
  Position *position = (Position *)p_state;
  while (position->x <= 15) {
    position->x++;
    position->y++;
    sleep(1);
  }

  return NULL;
}
