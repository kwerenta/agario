#include "draw.h"

#include "application.h"

int initialize_application(Application *app) {
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    printf("SDL_Init error: %s\n", SDL_GetError());
    return 0;
  }

  if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &app->window, &app->renderer) != 0) {
    SDL_Quit();
    printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
    return 0;
  };

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(app->renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
  SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);

  SDL_SetWindowTitle(app->window, "agar.io");

  app->screen =
      SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
  app->screenTexture = SDL_CreateTexture(app->renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                         SCREEN_WIDTH, SCREEN_HEIGHT);

  return 1;
}

void update_screen(Application *app) {
  SDL_UpdateTexture(app->screenTexture, NULL, app->screen->pixels, app->screen->pitch);
  SDL_RenderCopy(app->renderer, app->screenTexture, NULL, NULL);
  SDL_RenderPresent(app->renderer);
}

void render_players(Application *app, GameState *game) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Player player = game->players[i];

    if (player.color == 0)
      continue;

    u32 color =
        SDL_MapRGB(app->screen->format, player.color >> 24 & 0xFF, player.color >> 16 & 0xFF, player.color >> 8 & 0xFF);
    int size = 50 + player.score * 10;
    draw_rectangle(app->screen, player.position.x, player.position.y, size, size, color);
  }
}

void close_app(Application *app) {
  SDL_FreeSurface(app->screen);
  SDL_DestroyTexture(app->screenTexture);
  SDL_DestroyRenderer(app->renderer);
  SDL_DestroyWindow(app->window);

  SDL_Quit();
}
