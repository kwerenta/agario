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
  u8 player_id;
  Player players[MAX_PLAYERS];
  Position balls[MAX_BALLS];
} GameState;

void initialize_game_state(GameState *game);

void update_player_position(GameState *game);

#endif
