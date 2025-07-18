#include "WS2812Strip.hpp"

WS2812Strip::WS2812Strip(gpio_num_t gpio_pin, uint16_t led_count, spi_host_device_t spi_host)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_pin,
        .max_leds = led_count,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = {
            .format = {
                .r_pos = 1,
                .g_pos = 0,
                .b_pos = 2,
                .num_components = 3,
            },
        },
        .flags = {
            .invert_out = false,
        }
    };

    led_strip_spi_config_t spi_config = {
        .clk_src = SPI_CLK_SRC_DEFAULT,
        .spi_bus = spi_host,
        .flags = {
            .with_dma = true,
        }
    };

    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
    ESP_LOGI(TAG, "LED strip created on GPIO %d with %d LEDs", gpio_pin, led_count);
}

WS2812Strip::~WS2812Strip()
{
    if (led_strip) {
        led_strip_del(led_strip);
    }
}

esp_err_t WS2812Strip::set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b)
{
    return led_strip_set_pixel(led_strip, index, r, g, b);
}

esp_err_t WS2812Strip::refresh()
{
    return led_strip_refresh(led_strip);
}

esp_err_t WS2812Strip::clear()
{
    return led_strip_clear(led_strip);
}
