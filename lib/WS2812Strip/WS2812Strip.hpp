#pragma once

#include "led_strip.h"
#include "esp_err.h"
#include "esp_log.h"

class WS2812Strip {
public:
    WS2812Strip(gpio_num_t gpio_pin, uint16_t led_count, spi_host_device_t spi_host = SPI2_HOST);
    ~WS2812Strip();

    esp_err_t set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
    esp_err_t refresh();
    esp_err_t clear();

private:
    led_strip_handle_t led_strip;
    const char* TAG = "WS2812Strip";
};
