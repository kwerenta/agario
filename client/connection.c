#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../shared/config.h"
#include "../shared/serialization.h"

#include "connection.h"

const int setup_connection() {
  const int fd = socket(PF_INET, SOCK_STREAM, 0);

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(DEFAULT_PORT);

  char addrstr[NI_MAXHOST + NI_MAXSERV + 1];
  snprintf(addrstr, sizeof(addrstr), "127.0.0.1:%d", DEFAULT_PORT);

  inet_pton(AF_INET, addrstr, &addr.sin_addr);

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect error:");
    return -1;
  }

  return fd;
}

void *handle_connection(void *p_state) {
  u8 buffer[BUFFER_SIZE];
  State *state = (State *)p_state;

  while (recv(state->fd, buffer, sizeof(buffer), 0) > 0 && state->is_connected == 1) {
    state->game.player_id = buffer[5];

    for (u8 i = 0; i < (u8)buffer[2]; i++) {
      u8 id = buffer[6 + i * 17];

      deserialize_u32(state->game.players[id].color, buffer + 7 + i * 17);
      deserialize_f32(state->game.players[id].position.x, buffer + 11 + i * 17);
      deserialize_f32(state->game.players[id].position.y, buffer + 15 + i * 17);
      deserialize_u32(state->game.players[id].score, buffer + 19 + i * 17);
    }
  }

  state->is_connected = 0;

  return NULL;
}
