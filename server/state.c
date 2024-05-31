#include "state.h"

#include <stdio.h>

#include "../shared/random.h"
#include "../shared/utils.h"

u8 check_player_collision(Player *a, Player *b) {
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

void validate_move(Player *player, const ActionValue *action) {
  f32 distance = get_distance(action->position, player->position);
  f32 speed_multiplier = player->speed_time > 0 ? SPEED_MULTIPLIER : 1;

  // +0.1 to compensate any floating point arithmetic errors
  if (distance <= (get_player_speed(player->score) * speed_multiplier + 0.1)) {
    player->position = action->position;
  } else {
    printf("INCORRECT MOVE: Player %d, message %d (x=%f, y=%f)\n", action->player_id, action->message_id,
           action->position.x, action->position.y);
  }
};

void handle_player_collisions(State *state) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    for (int j = i + 1; j < MAX_PLAYERS; j++) {
      if (state->players[i].socket == 0 || state->players[j].socket == 0 || state->players[i].color == 0 ||
          state->players[j].color == 0)
        continue;

      if (check_player_collision(&state->players[i], &state->players[j]) == 1) {
        printf("Player %d has been killed by player %d\n", i, j);
        continue;
      }

      if (check_player_collision(&state->players[j], &state->players[i]) == 1)
        printf("Player %d has been killed by player %d\n", j, i);
    }
  }
};

void handle_ball_collisions(State *state) {
  for (int i = 0; i < MAX_BALLS; i++) {
    // Balls with position (0,0) are inactive
    if (state->balls[i].position.x == 0 && state->balls[i].position.y == 0)
      continue;

    for (int j = 0; j < MAX_PLAYERS; j++) {
      if (state->players[j].socket == 0 || state->players[j].color == 0)
        continue;

      float distance = get_distance(state->balls[i].position, state->players[j].position);

      // Check if ball is inside player
      if (distance + BALL_SIZE / 2.0 <= get_player_radius(state->players[j].score)) {
        state->players[j].score += BALL_SCORE;
        state->balls[i].position.x = 0;
        state->balls[i].position.y = 0;
        state->balls_count--;
      }
    }
  }
}

void handle_ball_spawn(State *state) {
  if (has_time_elapsed(&state->last_ball_spawn_time, sec_to_ms(BALL_SPAWN_TIME_SECONDS)) == 1 &&
      state->balls_count < MAX_BALLS) {

    for (int i = 0; i < MAX_BALLS; i++) {
      if (state->balls[i].position.x != 0 || state->balls[i].position.y != 0)
        continue;

      state->balls[i].position.x = random_range(BALL_SIZE, MAP_WIDTH - BALL_SIZE);
      state->balls[i].position.y = random_range(BALL_SIZE, MAP_HEIGHT - BALL_SIZE);
      state->balls_count++;
      break;
    }
  }
};

void handle_score_loss(State *state) {
  if (has_time_elapsed(&state->last_score_loss_time, sec_to_ms(SCORE_LOSS_TIME_SECONDS)) == 1) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
      if (state->players[i].socket == 0 || state->players[i].color == 0 || state->players[i].score == 0)
        continue;

      state->players[i].score *= SCORE_LOSS_MULTIPLIER;
    }
  }
}

void handle_speed_time(Player *player) {
  if (player->speed_time != 0) {
    i8 is_active = player->speed_time > 0 ? 1 : -1;
    player->speed_time -= is_active * sec_to_us(1.0 / TICKS_PER_SECOND);

    // Negative speed time means that it is on cooldown
    if (is_active == 1 && player->speed_time <= 0)
      player->speed_time = -sec_to_us(SPEED_COOLDOWN_SECONDS);
    else if (is_active == -1 && player->speed_time >= 0)
      player->speed_time = 0;

    // printf("Player %d speed time %d\n", i, state->players[i].speed_time);
  }
}
