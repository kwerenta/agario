#include "game.h"

void initialize_game_state(GameState *game) {
  for (int i = 0; i < MAX_PLAYERS; i++)
    game->players[i] = (Position){.x = 0, .y = 0};
}
