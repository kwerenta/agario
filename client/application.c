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

  initialize_colors(app);

  return 1;
}

void initialize_colors(Application *app) {
  app->colors[RED] = SDL_MapRGB(app->screen->format, 0xE9, 0x46, 0x39);
  app->colors[WHITE] = SDL_MapRGB(app->screen->format, 0xF2, 0xF2, 0xF6);
  app->colors[BLACK] = SDL_MapRGB(app->screen->format, 0x00, 0x00, 0x00);
}

void update_screen(Application *app) {
  SDL_UpdateTexture(app->screenTexture, NULL, app->screen->pixels, app->screen->pitch);
  SDL_RenderCopy(app->renderer, app->screenTexture, NULL, NULL);
  SDL_RenderPresent(app->renderer);
}

void render_players(Application *app, GameState *game) {
  for (int i = 0; i < MAX_PLAYERS; i++) {
    Color color = i % 2 ? RED : WHITE;
    draw_rectangle(app->screen, game->players[i].x * 10, game->players[i].y * 10, 100, 100, app->colors[color]);
  }
}

void close_app(Application *app) {
  SDL_FreeSurface(app->screen);
  SDL_DestroyTexture(app->screenTexture);
  SDL_DestroyRenderer(app->renderer);
  SDL_DestroyWindow(app->window);

  SDL_Quit();
}
