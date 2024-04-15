#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "application.h"
#include "connection.h"
#include "draw.h"

int main() {
  const int fd = setup_connection();

  if (fd < 0)
    return 1;

  Application app;
  SDL_Event event;
  int is_running = 1;

  State state = {.fd = fd, .is_connected = 1};
  initialize_game_state(&state.game);

  pthread_t connection_thread;
  pthread_create(&connection_thread, NULL, handle_connection, &state);

  initialize_application(&app);

  u8 message[2] = {0, 5};

  while (is_running) {
    // Close application if client lose connection with server
    if (state.is_connected == 0) {
      is_running = 0;
    }

    SDL_FillRect(app.screen, NULL, app.colors[BLACK]);
    render_players(&app, &state.game);
    update_screen(&app);

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_SPACE) {
          send(fd, message, sizeof(message), 0);
        }
      } else if (event.type == SDL_QUIT) {
        is_running = 0;
        state.is_connected = 0;
        break;
      }
    }
  }

  pthread_join(connection_thread, NULL);

  close_app(&app);
  close(fd);
  return 0;
}
