#include <SDL2/SDL.h>

#include "../shared/utils.h"

#include "game.h"

void initialize_game_state(GameState *game) {
  game->player_id = 0;
  for (int i = 0; i < MAX_PLAYERS; i++)
    game->players[i] = (Player){.color = 0, .position = {0}, .score = 0};

  for (int i = 0; i < MAX_BALLS; i++)
    game->balls[i] = (Position){.x = 0, .y = 0};

  game->speed_time = sec_to_us(DEFAULT_SPEED_TIME_SECONDS);
}

void update_player_position(GameState *game) {
  i32 mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);

  Player *player = &game->players[game->player_id];

  f32 dx = mouseX - player->position.x;
  f32 dy = mouseY - player->position.y;
  f32 distance = sqrt(dx * dx + dy * dy);

  if (distance < 1)
    return;

  if (distance != 0) {
    dx /= distance;
    dy /= distance;
  }

  f32 speed_multiplier = game->speed_time > 0 ? SPEED_MULTIPLIER : 1.0;

  player->position.x += dx * get_player_speed(player->score) * speed_multiplier;
  player->position.y += dy * get_player_speed(player->score) * speed_multiplier;
}
