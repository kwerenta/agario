#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "../shared/config.h"
#include "../shared/serialization.h"

#include "connection.h"

const int setup_connection(char *address, u16 port) {
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  if (inet_pton(AF_INET, address, &addr.sin_addr) <= 0) {
    perror("Invalid address");
    return -1;
  }

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect error:");
    return -1;
  }

  return fd;
}

void *handle_connection(void *p_state) {
  u8 buffer[BUFFER_SIZE];
  State *state = (State *)p_state;
  u32 color = 0;
  u16 balls_count = 0;
  u8 players_count = 0;

  u8 action;
  u16 message_id;

  while (recv(state->fd, buffer, sizeof(buffer), 0) > 0 && state->is_connected == 1) {
    deserialize_header(buffer, &action, &message_id);
    players_count = buffer[2];
    state->game.player_id = buffer[5];

    for (u8 i = 0; i < players_count; i++) {
      u8 id = buffer[6 + i * 17];

      deserialize_u32(state->game.players[id].color, buffer + 7 + i * 17);
      deserialize_u32(state->game.players[id].score, buffer + 19 + i * 17);

      if (id == state->game.player_id && color != 0 && state->game.players[id].color == 0) {
        printf("You died.\n");
        state->is_running = 0;
        state->is_connected = 0;
        return NULL;
      }

      // Client-side prediction (optimistic updates)
      if (id == state->game.player_id && state->action_queue != NULL) {
        color = state->game.players[id].color;
        // Client did not send new action between server responses
        if (state->last_message_id == message_id) {
          continue;
        }

        pthread_mutex_lock(state->action_queue_mutex);

        ActionValue action = {0};

        // Remove all old actions
        while (*state->action_queue != NULL && (*state->action_queue)->data.message_id <= message_id) {
          action = dequeue(state->action_queue);
        }

        Position received_position = {0};
        deserialize_f32(received_position.x, buffer + 11 + i * 17);
        deserialize_f32(received_position.y, buffer + 15 + i * 17);

        state->last_message_id = message_id;

        pthread_mutex_unlock(state->action_queue_mutex);

        // Skip updating position if move was correct
        if (action.position.x == received_position.x && action.position.y == received_position.y) {
          continue;
        }
      }

      deserialize_f32(state->game.players[id].position.x, buffer + 11 + i * 17);
      deserialize_f32(state->game.players[id].position.y, buffer + 15 + i * 17);
    }

    deserialize_u16(balls_count, buffer + 3);

    memset(state->game.balls, 0, MAX_BALLS);

    for (u16 i = 0; i < balls_count; i++) {
      deserialize_f32(state->game.balls[i].x, buffer + 6 + 17 * players_count + i * 8);
      deserialize_f32(state->game.balls[i].y, buffer + 10 + 17 * players_count + i * 8);
    }
  }

  state->is_connected = 0;

  return NULL;
}
