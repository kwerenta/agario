#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../shared/serialization.h"
#include "../shared/utils.h"

#include "application.h"
#include "connection.h"
#include "draw.h"
#include "game.h"

static void frame(Application *app, State *state, SDL_Event *event) {
  // Close application if client lose connection with server
  if (state->is_connected == 0) {
    state->is_running = 0;
  }

  u32 tick_time = SDL_GetTicks();
  app->clock.delta = tick_time - app->clock.last_tick;
  app->clock.last_tick = tick_time;

  SDL_SetRenderDrawColor(app->renderer, 33, 33, 33, 255);
  SDL_RenderClear(app->renderer);

  update_player_position(&state->game);
  render_players(app, &state->game);

  render_balls(app, &state->game);

  SDL_RenderPresent(app->renderer);

  while (SDL_PollEvent(event)) {
    if (event->type == SDL_KEYDOWN) {
      switch (event->key.keysym.sym) {
      case SDLK_SPACE: {
        if (state->game.speed_time != 0)
          continue;

        u8 speed_message[2];
        serialize_header(speed_message, 2, state->last_message_id);
        state->last_message_id++;
        send(state->fd, speed_message, sizeof(speed_message), 0);
        state->game.speed_time = sec_to_us(SPEED_TIME_SECONDS);
        break;
      }
      }
    }

    if (event->type == SDL_QUIT) {
      state->is_running = 0;
      state->is_connected = 0;
      break;
    }
  }

  if (state->game.speed_time != 0) {
    i8 is_active = state->game.speed_time > 0 ? 1 : -1;
    state->game.speed_time -= is_active * ms_to_us(app->clock.delta);

    if (is_active == 1 && state->game.speed_time <= 0)
      state->game.speed_time = -sec_to_us(SPEED_COOLDOWN_SECONDS);
    else if (is_active == -1 && state->game.speed_time >= 0)
      state->game.speed_time = 0;

    // printf("Player speed time %d\n", state->game.speed_time);
  }

  // Fixed 60FPS
  SDL_Delay(16);

  pthread_mutex_lock(state->action_queue_mutex);
  // TEMP: sending message directly after frame render
  u8 move_message[10];
  serialize_header(move_message, 1, state->last_message_id);
  serialize_f32(move_message + 2, state->game.players[state->game.player_id].position.x);
  serialize_f32(move_message + 6, state->game.players[state->game.player_id].position.y);
  send(state->fd, move_message, sizeof(move_message), 0);

  enqueue(state->action_queue, (ActionValue){.player_id = state->game.player_id,
                                             .message_id = state->last_message_id,
                                             .position = state->game.players[state->game.player_id].position});

  // TODO: Handle message id overflow
  state->last_message_id++;
  // 12 bit message id
  state->last_message_id %= 0x1000;

  pthread_mutex_unlock(state->action_queue_mutex);
}

int main(int argc, char **argv) {
  u32 port = DEFAULT_PORT;

  if (argc == 3) {
    port = get_port_from_string(argv[2]);
    if (port == 0) {
      perror("Invalid port");
      return 1;
    }
  }
  const int fd = setup_connection(argc > 1 ? argv[1] : "127.0.0.1", port);

  if (fd < 0)
    return 1;

  Application app;
  SDL_Event event;

  ActionNode *action_queue = NULL;

  pthread_mutex_t action_queue_mutex;

  if (pthread_mutex_init(&action_queue_mutex, NULL) != 0) {
    perror("Failed to init action queue mutex");
    return 1;
  }

  State state = {.fd = fd,
                 .is_connected = 1,
                 .is_running = 1,
                 .last_message_id = 1,
                 .action_queue = &action_queue,
                 .action_queue_mutex = &action_queue_mutex};
  initialize_game_state(&state.game);

  pthread_t connection_thread;
  pthread_create(&connection_thread, NULL, handle_connection, &state);

  initialize_application(&app);

  // Send join message to server
  u8 buffer[6] = {0};
  serialize_header(buffer, 0, 0);

  // TEMP: Color selected randomly
  srand(time(NULL));
  u32 color = (1 + rand() / ((RAND_MAX + 1u) / 0xFFFFFF)) << 8;

  serialize_u32(buffer + 2, color);

  send(fd, buffer, sizeof(buffer), 0);
  printf("Client sent JOIN message to server\n");

  while (state.is_running) {
    frame(&app, &state, &event);
  }

  pthread_join(connection_thread, NULL);

  pthread_mutex_destroy(&action_queue_mutex);

  close_app(&app);
  close(fd);
  return 0;
}
