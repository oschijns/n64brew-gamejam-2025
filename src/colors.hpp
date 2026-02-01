#pragma once
#include "graphics.h"
#define RED    (color_t){0xFF, 0x00, 0x00, 0xFF}
#define BLUE   (color_t){0x00, 0xFF, 0x00, 0xFF}
#define GREEN  (color_t){0x00, 0x00, 0xFF, 0xFF}
#define YELLOW (color_t){0xFF, 0xFF, 0x00, 0xFF}
const color_t COLORS[4] {
RED,
BLUE,
GREEN,
YELLOW
};

inline bool color_equals(const color_t c1, const color_t c2) {
  return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}
