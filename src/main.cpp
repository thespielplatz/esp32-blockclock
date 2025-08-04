#include <stdio.h>
#include <WS2812Strip.hpp>
#include <Display.hpp>
#include <PixelBounceAnimation.hpp>
#include <BlockHeightFetcher.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "NVSStore.hpp"

#include <ImprovWifiManager.hpp>

static const char *TAG = "main";

extern "C" {
    void app_main(void);
}

void showError(Display& display, const char* message) {
    display.clear();
    display.setColor(255, 0, 0);
    display.writeText(0, message, true);
    display.render();
    vTaskDelay(pdMS_TO_TICKS(10));
}

extern "C" void app_main() {
    ImprovWifiManager wifiManager({
        "ESP32-Blockclock",  // firmware name
        "1.0.0",             // firmware version
        "ESP32",             // hardware variant
        "Blockclock"         // device name
    });

    uint16_t width = 50;
    uint16_t height = 5;

    WS2812Strip strip(GPIO_NUM_18, width * height);
    Display display(strip, width, height, true);

    if (NVSStore::initNvsFlash() != true) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        showError(display, "NVS Init");
        vTaskDelay(pdMS_TO_TICKS(10000));
        return;
    }

    display.clear();
    display.setColor(100, 100, 100);
    display.writeText(0, "Connecting", true);
    display.render();
    vTaskDelay(pdMS_TO_TICKS(10));

    wifiManager.begin();

    PixelBounceAnimation anim(display, 4, 250, 125, 0, 50, 1000);
    BlockHeightFetcher fetcher;

    while (true) {
        display.clear();

        anim.update();
        fetcher.update();

        display.setColor(100, 100, 100);
        display.writeText(0, fetcher.getText(), true);
        display.render();

        wifiManager.loop();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}