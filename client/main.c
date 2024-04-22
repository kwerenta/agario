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
#include "game.h"

static void frame(Application *app, State *state, SDL_Event *event) {
  // Close application if client lose connection with server
  if (state->is_connected == 0) {
    state->is_running = 0;
  }

  SDL_SetRenderDrawColor(app->renderer, 33, 33, 33, 255);
  SDL_RenderClear(app->renderer);

  update_player_position(&state->game);
  render_players(app, &state->game);

  SDL_RenderPresent(app->renderer);

  while (SDL_PollEvent(event)) {
    if (event->type == SDL_KEYDOWN) {
      switch (event->key.keysym.sym) {
      case SDLK_SPACE:
        send(state->fd, (u8[1]){0b10000000}, sizeof(u8), 0);
        break;
      }
    }

    if (event->type == SDL_QUIT) {
      state->is_running = 0;
      state->is_connected = 0;
      break;
    }
  }

  // Fixed 60FPS
  SDL_Delay(16);

  // TEMP: sending message directly after frame render
  u8 move_message[9];
  move_message[0] = 0b01000000;
  *(f32 *)(move_message + 1) = state->game.players[state->game.player_id].position.x;
  *(f32 *)(move_message + 5) = state->game.players[state->game.player_id].position.y;
  send(state->fd, move_message, sizeof(move_message), 0);
}

int main() {
  const int fd = setup_connection();

  if (fd < 0)
    return 1;

  Application app;
  SDL_Event event;

  State state = {.fd = fd, .is_connected = 1, .is_running = 1};
  initialize_game_state(&state.game);

  pthread_t connection_thread;
  pthread_create(&connection_thread, NULL, handle_connection, &state);

  initialize_application(&app);

  // Send join message to server
  u8 buffer[5] = {0};

  // TEMP: Color selected randomly
  srand(time(NULL));
  u32 color = (1 + rand() / ((RAND_MAX + 1u) / 0xFFFFFF)) << 8;

  *(u32 *)(buffer + 1) = htonl(color);

  send(fd, buffer, sizeof(buffer), 0);
  printf("Client sent JOIN message to server\n");

  while (state.is_running) {
    frame(&app, &state, &event);
  }

  pthread_join(connection_thread, NULL);

  close_app(&app);
  close(fd);
  return 0;
}
