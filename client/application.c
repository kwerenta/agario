#include "../shared/utils.h"
#include "draw.h"

#include "application.h"

int initialize_application(Application *app) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    printf("SDL_Init error: %s\n", SDL_GetError());
    return 0;
  }

  app->window = SDL_CreateWindow("agar.io", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                 SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

  if (app->window == NULL) {
    SDL_Quit();
    printf("Failed to create window: %s\n", SDL_GetError());
    return 0;
  }

  app->renderer = SDL_CreateRenderer(app->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (app->renderer == NULL) {
    SDL_DestroyWindow(app->window);
    SDL_Quit();
    printf("Failed to create renderer: %s\n", SDL_GetError());
    return 0;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(app->renderer, 640, 480);

  app->clock = (Clock){.delta = 0, .last_tick = 0};

  return 1;
}

void render_players(Application *app, GameState *game) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = &game->players[i];

    if (player->color == 0)
      continue;

    u32 size = get_player_radius(player->score);

    draw_circle(app->renderer, player->position.x, player->position.y, size, player->color);
  }
}

void render_balls(Application *app, GameState *game) {
  for (int i = 0; i < MAX_BALLS; i++) {
    if (game->balls[i].x == 0 && game->balls[i].y == 0)
      continue;
    // draw_circle(app->renderer, game->balls[i].x, game->balls[i].y, 8, 0x00FF0000);
    draw_rectangle(app->renderer, game->balls[i].x, game->balls[i].y, BALL_SIZE, BALL_SIZE, 0x00FF0000);
  }
}

void close_app(Application *app) {
  SDL_DestroyRenderer(app->renderer);
  SDL_DestroyWindow(app->window);

  SDL_Quit();
}
