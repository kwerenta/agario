#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "draw.h"
#include "ui.h"

#define PORT 2002

typedef struct SocketWorkerArgs {
  int fd;
  int x;
  int y;
  int is_running;
} SocketWorkerArgs;

void *socket_worker(void *worker_args) {
  char buffer[1024];
  SocketWorkerArgs *args = ((SocketWorkerArgs *)worker_args);

  while (recv(args->fd, buffer, sizeof(buffer), 0) > 0) {
    if (sscanf(buffer, "%d,%d", &args->x, &args->y) != 2)
      break;
  }

  args->is_running = 0;

  return NULL;
}

int main() {
  int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons((short)PORT);

  char addrstr[NI_MAXHOST + NI_MAXSERV + 1];
  snprintf(addrstr, sizeof(addrstr), "127.0.0.1:%d", PORT);

  inet_pton(AF_INET, addrstr, &addr.sin_addr);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect error:");
    return 1;
  }

  App app;
  SDL_Event event;
  int is_running = 1;

  SocketWorkerArgs worker_args = {.fd = fd, .x = 100, .y = 100, .is_running = 1};

  pthread_t thread;
  pthread_create(&thread, NULL, socket_worker, &worker_args);

  init_app(&app);

  while (is_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        is_running = 0;
    }

    if (worker_args.is_running == 0) {
      is_running = 0;
    }

    SDL_FillRect(app.screen, NULL, app.colors[BLACK]);
    draw_rectangle(app.screen, worker_args.x * 20, worker_args.y * 20, 100, 100, app.colors[RED]);
    update_screen(&app);
  }

  pthread_join(thread, NULL);

  close_app(&app);
  close(fd);
  return 0;
}
