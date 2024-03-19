#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "draw.h"
#include "ui.h"

#define PORT 2002

int main() {
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

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

  int x = 100, y = 100;
  char buffer[1024];

  init_app(&app);

  while (is_running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        is_running = 0;
    }

    SDL_FillRect(app.screen, NULL, app.colors[BLACK]);
    draw_rectangle(app.screen, x, y, 100, 100, app.colors[RED]);
    update_screen(&app);

    if (recv(fd, buffer, sizeof(buffer), 0) > 0) {
      sscanf(buffer, "%d,%d", &x, &y);
    } else {
      is_running = 0;
    }
  }

  close_app(&app);
  close(fd);
  return 0;
}
