#include "utils.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

float get_distance(Position a, Position b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
};

u32 get_player_radius(u32 score) { return 5 * score + 20; }

float get_player_speed(u32 score) { return 1 + 1.0 / (score + 1); }

u16 get_port_from_string(char *str) {
  for (int i = 0; i < strlen(str); i++)
    if (isdigit(str[i]) == 0) {
      return 0;
    }

  u32 port = atoi(str);
  if (port != (port & 0xFFFF)) {
    return 0;
  }

  return port;
}
