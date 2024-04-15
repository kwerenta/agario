#include <SDL2/SDL.h>

#include "game.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define COLORS_COUNT 3

typedef enum Color { RED, WHITE, BLACK } Color;

typedef struct Application {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Surface *screen;
  SDL_Texture *screenTexture;
  u32 colors[COLORS_COUNT];
} Application;

int initialize_application(Application *app);
void initialize_colors(Application *app);

void update_screen(Application *app);
void render_players(Application *app, GameState *game);

void close_app(Application *app);
