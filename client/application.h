#include <SDL2/SDL.h>

#include "game.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

typedef struct Application {
  SDL_Renderer *renderer;
  SDL_Window *window;
} Application;

int initialize_application(Application *app);

void render_players(Application *app, GameState *game);

void close_app(Application *app);
