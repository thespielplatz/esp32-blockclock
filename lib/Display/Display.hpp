#pragma once

#include "WS2812Strip.hpp"

class Display {
public:
    Display(WS2812Strip& strip, uint16_t width, uint16_t height, bool row_inversion = true);
    esp_err_t set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
    esp_err_t fill(uint8_t r, uint8_t g, uint8_t b);
    esp_err_t clear();    esp_err_t refresh();

private:
    WS2812Strip& strip;
    uint16_t width;
    uint16_t height;
    bool row_inversion;

    uint16_t to_strip_index(uint16_t x, uint16_t y) const;
};
