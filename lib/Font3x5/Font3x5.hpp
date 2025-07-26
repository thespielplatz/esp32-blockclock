#pragma once
#include <stdint.h>

struct FontChar {
    char character;
    uint8_t pixels[5]; // 5 columns of 3 bits (3x5)
};

extern const FontChar FONT_3x5[];
extern const int FONT_3x5_LENGTH;

const uint8_t* get_char_pixels(char c);
