#pragma once

#include "WS2812Strip.hpp"
#include <vector>
#include <array>
#include <string>

class Display {
public:
    Display(WS2812Strip& strip, uint16_t width, uint16_t height, bool row_inversion = true);
    
    void set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b);
    void fill(uint8_t r, uint8_t g, uint8_t b);
    void clear();
    esp_err_t render();

    void setColor(uint8_t r, uint8_t g, uint8_t b);
    esp_err_t writeText(int offset, const std::string& text, bool centered = false);

private:
    WS2812Strip& strip;
    uint16_t width;
    uint16_t height;
    bool row_inversion;

    std::vector<std::array<uint8_t, 3>> framebuffer;
    std::array<uint8_t, 3> currentColor = {255, 255, 255};  // Default to white

    uint16_t to_strip_index(uint16_t x, uint16_t y) const;
    uint16_t to_framebuffer_index(uint16_t x, uint16_t y) const;
};
