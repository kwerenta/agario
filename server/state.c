#include "state.h"

#include "../shared/utils.h"

u8 handle_player_collision(Player *a, Player *b) {
  float distance = get_distance(a->position, b->position);

  u32 a_radius = get_player_radius(a->score);
  u32 b_radius = get_player_radius(b->score);

  if (distance + a_radius <= b_radius) {
    b->score += a->score;

    a->score = 0;
    a->color = 0;
    return 1;
  }

  return 0;
}
