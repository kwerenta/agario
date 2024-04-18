#include <SDL2/SDL.h>

#include "game.h"

void initialize_game_state(GameState *game) {
  game->player_id = 0;
  for (int i = 0; i < MAX_PLAYERS; i++)
    game->players[i] = (Player){.color = 0, .position = {0}, .score = 0};
}

void update_player_position(GameState *game) {
  i32 mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);

  Player *player = &game->players[game->player_id];

  float dx = mouseX - player->position.x;
  float dy = mouseY - player->position.y;
  float distance = sqrt(dx * dx + dy * dy);

  if (distance != 0) {
    dx /= distance;
    dy /= distance;
  }

  player->position.x += dx * (1 + 1.0 / (player->score + 1));
  player->position.y += dy * (1 + 1.0 / (player->score + 1));
}
