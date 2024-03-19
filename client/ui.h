#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define COLORS_COUNT 3

typedef enum Colors { RED, WHITE, BLACK } Colors;

typedef struct App {
  SDL_Renderer *renderer;
  SDL_Window *window;
  SDL_Surface *screen;
  SDL_Texture *screenTexture;
  Uint32 colors[COLORS_COUNT];
} App;

int init_app(App *app);
void update_screen(App *app);
void init_colors(App *app);
void close_app(App *app);
