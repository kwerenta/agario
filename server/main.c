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

typedef struct client {
  int cfd;
  pthread_t thread;
} client_t;

void *handle_connection(void *cfd);

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

  while (client_count < MAX_PLAYERS) {
    int cfd = accept(fd, (struct sockaddr *)&caddr, &caddr_len);
    clients[client_count].cfd = cfd;

    if (pthread_create(&clients[client_count].thread, NULL, handle_connection, &cfd)) {
      perror("failed to create thread for client");
    }

    client_count++;
  }

  for (int i = 0; i < MAX_PLAYERS; i++) {
    pthread_join(clients[i].thread, NULL);
    close(clients[i].cfd);
  }

  close(fd);
}

void *handle_connection(void *cfd_ptr) {
  int cfd = *((int *)cfd_ptr);

  char buf[1024];
  int x = 0, y = 0;

  while (1) {
    snprintf(buf, sizeof(buf), "%d,%d", x, y);
    send(cfd, buf, strlen(buf) + 1, 0);
    sleep(1);
    x += 10;
    y += 10;

    if (y >= 150)
      break;
  }

  return NULL;
}
