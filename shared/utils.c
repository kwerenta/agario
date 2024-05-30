#include "utils.h"

#include <math.h>

float get_distance(Position a, Position b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
};

u32 get_player_radius(u32 score) { return 5 * score + 20; }

float get_player_speed(u32 score) { return 1 + 1.0 / (score + 1); };
