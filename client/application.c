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

  return 1;
}

void render_players(Application *app, GameState *game) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player *player = &game->players[i];

    if (player->color == 0)
      continue;

    u32 size = 20 + player->score * 5;

    draw_circle(app->renderer, player->position.x, player->position.y, size, player->color);
  }
}

void close_app(Application *app) {
  SDL_DestroyRenderer(app->renderer);
  SDL_DestroyWindow(app->window);

  SDL_Quit();
}
