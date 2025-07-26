#include "Font3x5.hpp"

// Define characters as pixel columns, each 3 bits high
const FontChar FONT_3x5[] = {
    {'A', {0b111, 0b101, 0b111, 0b101, 0b101}},
    {'B', {0b110, 0b101, 0b110, 0b101, 0b110}},
    {'C', {0b111, 0b100, 0b100, 0b100, 0b111}},
    {'H', {0b101, 0b101, 0b111, 0b101, 0b101}},
    {'I', {0b111, 0b010, 0b010, 0b010, 0b111}},
    {'E', {0b111, 0b100, 0b110, 0b100, 0b111}},
    {'L', {0b100, 0b100, 0b100, 0b100, 0b111}},
    {'O', {0b111, 0b101, 0b101, 0b101, 0b111}},
    {' ', {0b000, 0b000, 0b000, 0b000, 0b000}},
};

const int FONT_3x5_LENGTH = sizeof(FONT_3x5) / sizeof(FontChar);

const uint8_t* get_char_pixels(char c)
{
    for (int i = 0; i < FONT_3x5_LENGTH; ++i) {
        if (FONT_3x5[i].character == c) {
            return FONT_3x5[i].pixels;
        }
    }
    return FONT_3x5[FONT_3x5_LENGTH - 1].pixels; // fallback: space
}
