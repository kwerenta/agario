#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>

#include "../shared/config.h"

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
    state->game.player_id = buffer[4];

    for (u8 i = 0; i < (u8)buffer[1]; i++) {
      u8 id = buffer[5 + i * 17];
      state->game.players[id].color = ntohl(*(u32 *)(buffer + 6 + i * 17));
      state->game.players[id].position.x = *(f32 *)(buffer + 10 + i * 17);
      state->game.players[id].position.y = *(f32 *)(buffer + 14 + i * 17);
      state->game.players[id].score = ntohl(*(u32 *)(buffer + 18 + i * 17));
    }
  }

  state->is_connected = 0;

  return NULL;
}
