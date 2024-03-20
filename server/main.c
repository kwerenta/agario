#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 2002
#define MAX_PLAYERS 2

typedef struct Position {
  int x;
  int y;
} Position;

typedef struct client {
  int cfd;
  Position *position;
  pthread_t thread;
} client_t;

void *handle_connection(void *p_client);
void *update_position(void *p_position);

int main() {
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(SERVER_PORT);

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("bind error:");
    return 1;
  }

  socklen_t addr_len = sizeof(addr);
  getsockname(fd, (struct sockaddr *)&addr, &addr_len);
  printf("Server is listening on port %d\n", (int)ntohs(addr.sin_port));

  if (listen(fd, 1)) {
    perror("listen error:");
    return 1;
  }

  struct sockaddr_storage caddr;
  socklen_t caddr_len = sizeof(caddr);

  client_t clients[MAX_PLAYERS] = {};
  int client_count = 0;

  Position position = {.x = 0, .y = 0};

  while (client_count < MAX_PLAYERS) {
    int cfd = accept(fd, (struct sockaddr *)&caddr, &caddr_len);
    clients[client_count].cfd = cfd;
    clients[client_count].position = &position;

    if (pthread_create(&clients[client_count].thread, NULL, handle_connection, &clients[client_count])) {
      perror("failed to create thread for client");
    }

    printf("client %d connected\n", client_count);
    client_count++;
  }

  pthread_t position_thread;
  pthread_create(&position_thread, NULL, update_position, &position);
  pthread_join(position_thread, NULL);

  for (int i = 0; i < MAX_PLAYERS; i++) {
    pthread_join(clients[i].thread, NULL);
    close(clients[i].cfd);
  }

  close(fd);
}

void *update_position(void *p_position) {
  Position *position = (Position *)p_position;
  while (position->x <= 15) {
    position->x++;
    position->y++;
    sleep(1);
  }

  return NULL;
}

void *handle_connection(void *p_client) {
  client_t client = *((client_t *)p_client);

  char buf[1024];

  while (1) {
    snprintf(buf, sizeof(buf), "P0,%d,%d,P1,%d,%d", client.position->x, client.position->y, 15 - client.position->x,
             10 - client.position->y);
    send(client.cfd, buf, strlen(buf) + 1, 0);
    sleep(1);

    if (client.position->x >= 15)
      break;
  }

  printf("stopped handling connection\n");
  return NULL;
}
