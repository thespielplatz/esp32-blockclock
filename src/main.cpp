#include <stdio.h>
#include <WS2812Strip.hpp>
#include <Display.hpp>
#include <PixelBounceAnimation.hpp>
#include <BlockHeightFetcher.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "secrets.h"
#include "wifi_connect.hpp"

extern "C" {
    void app_main(void);
}

extern "C" void app_main() {
    uint16_t width = 50;
    uint16_t height = 5;

    WS2812Strip strip(GPIO_NUM_18, width * height);
    Display display(strip, width, height, true);

    display.clear();
    display.setColor(100, 100, 100);
    display.writeText(0, WIFI_SSID, true);
    display.render();
    vTaskDelay(pdMS_TO_TICKS(10));

    wifi_connect();  // Blocks until Wi-Fi is up

    PixelBounceAnimation anim(display, 4, 250, 125, 0, 50, 1000);
    BlockHeightFetcher fetcher;

    while (true) {
        display.clear();

        anim.update();
        fetcher.update();

        display.setColor(100, 100, 100);
        display.writeText(0, fetcher.getText(), true);
        display.render();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}