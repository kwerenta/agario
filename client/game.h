#ifndef GAME_H
#define GAME_H

#include "../shared/config.h"
#include "../shared/types.h"

typedef struct GameState {
  Position players[MAX_PLAYERS];
} GameState;

void initialize_game_state(GameState *game);

#endif
