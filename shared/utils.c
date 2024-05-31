#include "utils.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

f32 get_distance(Position a, Position b) {
  float dx = a.x - b.x;
  float dy = a.y - b.y;
  return sqrt(dx * dx + dy * dy);
};

u32 get_player_radius(u32 score) { return 5 * score + 20; }

f32 get_player_speed(u32 score) { return 1 + 1.0 / (score + 1); }

u16 get_port_from_string(char *str) {
  for (int i = 0; i < strlen(str); i++)
    if (isdigit(str[i]) == 0) {
      return 0;
    }

  u32 port = atoi(str);
  if (port != (port & 0xFFFF)) {
    return 0;
  }

  return port;
}

u8 has_time_elapsed(struct timespec *last_time, u32 duration_ms) {
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);

  i32 elapsed_seconds = current_time.tv_sec - last_time->tv_sec;
  i32 elapsed_nanoseconds = current_time.tv_nsec - last_time->tv_nsec;

  // Convert elapsed time to milliseconds
  i32 elapsed_milliseconds = elapsed_seconds * 1000 + elapsed_nanoseconds / 1000000;

  if (elapsed_milliseconds >= duration_ms) {
    *last_time = current_time;
    return 1;
  }

  return 0;
};
