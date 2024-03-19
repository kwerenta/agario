#include "draw.h"

void draw_rectangle(SDL_Surface *screen, int x, int y, int w, int h, Uint32 color) {
  SDL_Rect rect = {.x = x, .y = y, .w = w, .h = h};
  SDL_FillRect(screen, &rect, color);
}
