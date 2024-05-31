#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#define sec_to_ms(sec) sec * 1000
#define sec_to_us(sec) sec * 1000 * 1000
#define sec_to_ns(sec) sec * 1000 * 1000 * 1000
#define ms_to_us(ms) ms * 1000

f32 get_distance(Position a, Position b);
u32 get_player_radius(u32 score);
f32 get_player_speed(u32 score);

u16 get_port_from_string(char *str);

#endif
