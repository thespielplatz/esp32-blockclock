#include <stdio.h>
#include <WS2812Strip.hpp>
#include <Display.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C" {
    void app_main(void);
}

void app_main() {
    uint16_t width = 50;
    uint16_t height = 5;

    // Configure WS2812 strip with 5 rows and 50 columns
    WS2812Strip strip(GPIO_NUM_18, height * width);
    Display display(strip, width, height, true); // true = serpentine layout

    uint16_t frame = 0;

    while (true) {
        display.clear();

        // Draw one vertical bar across all rows at position `frame`
        uint16_t x = frame % width;
        for (uint16_t y = 0; y < height; ++y) {
            display.set_pixel(x, y, 0, 200, 20);  // soft greenish bar
        }

        display.refresh();
        frame++;

        vTaskDelay(pdMS_TO_TICKS(100));  // wait 100ms
    }
}