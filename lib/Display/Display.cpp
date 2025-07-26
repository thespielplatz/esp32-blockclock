#include "Display.hpp"

Display::Display(WS2812Strip& strip, uint16_t width, uint16_t height, bool row_inversion)
    : strip(strip), width(width), height(height), row_inversion(row_inversion)
{
}

uint16_t Display::to_strip_index(uint16_t x, uint16_t y) const
{
    if (row_inversion && (y % 2 == 1)) {
        x = width - 1 - x;  // Invert the x index for odd rows
    }
    return y * width + x;
}

esp_err_t Display::set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b)
{
    if (x >= width || y >= height) return ESP_ERR_INVALID_ARG;
    return strip.set_pixel(to_strip_index(x, y), r, g, b);
}

esp_err_t Display::fill(uint8_t r, uint8_t g, uint8_t b)
{
    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            esp_err_t err = set_pixel(x, y, r, g, b);
            if (err != ESP_OK) return err;
        }
    }
    return ESP_OK;
}

esp_err_t Display::clear()
{
    return strip.clear();
}

esp_err_t Display::refresh()
{
    return strip.refresh();
}
