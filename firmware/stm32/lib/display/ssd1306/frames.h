#ifndef SSD1306_FRAMES_H
#define SSD1306_FRAMES_H

#include <stdint.h>
#include "ssd1306.h"

#define SSD1306_ANIM_FRAME_COUNT 24

/* One entry per frame, each SSD1306_BUFFER_SIZE bytes, laid out for
 * Horizontal Addressing Mode (page-major: page0 col0..127, page1 col0..127, ...). */
extern const uint8_t ssd1306_anim_frames[SSD1306_ANIM_FRAME_COUNT][SSD1306_BUFFER_SIZE];
extern const uint8_t bitmap[SSD1306_BUFFER_SIZE];

#endif /* SSD1306_FRAMES_H */