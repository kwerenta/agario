#include "random.h"

#include <stdlib.h>

u32 random_range(u32 min, u32 max) { return min + rand() / (RAND_MAX / (max - min + 1) + 1); }
