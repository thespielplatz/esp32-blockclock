#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"


extern "C" {
    void app_main(void);
}

void app_main() {
    const char* TAG = "HelloApp";

    while (true) {
        ESP_LOGI(TAG, "Hello, world!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}