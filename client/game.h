#ifndef GAME_H
#define GAME_H

#include "../shared/config.h"
#include "../shared/types.h"

typedef struct {
  u32 color;
  Position position;
  u32 score;
} Player;

typedef struct GameState {
  Player players[MAX_PLAYERS];
} GameState;

void initialize_game_state(GameState *game);

#endif
