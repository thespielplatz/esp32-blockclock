#include "Display.hpp"
#include "Font3x5.hpp"

Display::Display(WS2812Strip& strip, uint16_t width, uint16_t height, bool row_inversion)
    : strip(strip), width(width), height(height), row_inversion(row_inversion),
      framebuffer(width * height, {0, 0, 0})
{}

uint16_t Display::to_strip_index(uint16_t x, uint16_t y) const {
    if (row_inversion && (y % 2 == 1)) {
        x = width - 1 - x;
    }
    return y * width + x;
}

uint16_t Display::to_framebuffer_index(uint16_t x, uint16_t y) const {
    return y * width + x;
}

void Display::set_pixel(uint16_t x, uint16_t y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= width || y >= height) return;
    framebuffer[to_framebuffer_index(x, y)] = {r, g, b};
}

void Display::fill(uint8_t r, uint8_t g, uint8_t b) {
    for (auto& px : framebuffer) {
        px = {r, g, b};
    }
}

esp_err_t Display::writeText(int offset, const std::string& text, bool centered)
{
    const int CHAR_WIDTH = 3;
    const int CHAR_HEIGHT = 5;
    const int CHAR_SPACING = 1;
    const int CHAR_TOTAL_WIDTH = CHAR_WIDTH + CHAR_SPACING;

    int total_pixel_width = text.length() * CHAR_TOTAL_WIDTH - CHAR_SPACING;

    int start_x = offset;
    if (centered) {
        if (total_pixel_width > width) return ESP_ERR_INVALID_ARG;
        start_x = (width - total_pixel_width) / 2 + offset;
    }

    for (size_t i = 0; i < text.length(); ++i) {
        const uint8_t* col_data = get_char_pixels(text[i]);
        int char_x = start_x + i * CHAR_TOTAL_WIDTH;

        for (int col = 0; col < CHAR_WIDTH; ++col) {
            if (char_x + col < 0 || char_x + col >= width) continue;
            for (int row = 0; row < CHAR_HEIGHT; ++row) {
                uint8_t bits = col_data[row];  // bits for that row (i.e., bits for columns)

                for (int col = 0; col < CHAR_WIDTH; ++col) {
                    if ((bits >> (CHAR_WIDTH - 1 - col)) & 0x01) {
                        set_pixel(char_x + col, row, 100, 100, 100);  // Light gray
                    }
                }
            }
        }
    }

    return ESP_OK;
}

void Display::clear() {
    fill(0, 0, 0);
}

esp_err_t Display::render() {
    for (uint16_t y = 0; y < height; ++y) {
        for (uint16_t x = 0; x < width; ++x) {
            auto& px = framebuffer[to_framebuffer_index(x, y)];
            strip.set_pixel(to_strip_index(x, y), px[0], px[1], px[2]);
        }
    }
    return strip.refresh();
}
