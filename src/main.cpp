#include <stdio.h>
#include <WS2812Strip.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

extern "C" {
    void app_main(void);
}

void app_main() {
    WS2812Strip strip(GPIO_NUM_18, 8);

    const char* TAG = "HelloApp";
    bool on = false;

    while (true) {
        ESP_LOGI(TAG, "Hello, world!");
        if (on) {
            for (int i = 0; i < 24; i++) {
                strip.set_pixel(i, 5, 5, 5);
            }
            strip.refresh();
        } else {
            strip.clear();
        }

        on = !on;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}