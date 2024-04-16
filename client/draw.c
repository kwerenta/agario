#include "draw.h"

void draw_rectangle(SDL_Renderer *renderer, i32 x, i32 y, i32 w, i32 h, u32 color) {
  SDL_Rect rect = {.x = x, .y = y, .w = w, .h = h};
  SDL_SetRenderDrawColor(renderer, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, 255);
  SDL_RenderFillRect(renderer, &rect);
}

void draw_circle(SDL_Renderer *renderer, i32 x, i32 y, u32 radius, u32 color) {
  SDL_SetRenderDrawColor(renderer, (color >> 24) & 0xFF, (color >> 16) & 0xFF, (color >> 8) & 0xFF, 255);

  for (int w = 0; w < radius * 2; w++) {
    for (int h = 0; h < radius * 2; h++) {
      int dx = radius - w; // horizontal offset
      int dy = radius - h; // vertical offset
      if ((dx * dx + dy * dy) <= (radius * radius)) {
        SDL_RenderDrawPoint(renderer, x + dx, y + dy);
      }
    }
  }
}
