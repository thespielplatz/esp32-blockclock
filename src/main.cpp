#include <stdio.h>
#include <WS2812Strip.hpp>
#include <Display.hpp>
#include <PixelBounceAnimation.hpp>
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

    PixelBounceAnimation anim(display, 4, 250, 125, 0, 50, 1000);

    uint16_t frame = 0;

    while (true) {
        display.clear();

        anim.update();

        display.setColor(100, 100, 100);
        display.writeText(0, "123456", true);

        display.render();
        frame++;

        vTaskDelay(pdMS_TO_TICKS(10));  // wait 10ms
    }
}