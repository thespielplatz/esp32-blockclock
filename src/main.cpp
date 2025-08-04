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

#include <ImprovManager.hpp>
#include <WifiManager.hpp>

static const char *TAG = "main";

extern "C" {
    void app_main(void);
}

void showMessage(Display& display, const char* message) {
    display.clear();
    display.setColor(0, 0, 250);
    display.writeText(0, message, true);
    display.render();
    vTaskDelay(pdMS_TO_TICKS(10));
}

void improvLoop(Display& display, ImprovManager& improvManager, const char* error) {
    display.clear();
    display.setColor(255, 0, 0);
    display.writeText(0, error, true);
    display.render();
    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "Improv Manager Loop");
    showMessage(display, "Improv Manager");
    while (true) {
        improvManager.loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }    
}

extern "C" void app_main() {
    WifiManager wifiManager;
    ImprovManager improvManager({
        "ESP32-Blockclock",  // firmware name
        "1.0.0",             // firmware version
        "ESP32",             // hardware variant
        "Blockclock"         // device name
    }, wifiManager);

    uint16_t width = 50;
    uint16_t height = 5;

    WS2812Strip strip(GPIO_NUM_18, width * height);
    Display display(strip, width, height, true);

    if (NVSStore::initNvsFlash() != true) {
        ESP_LOGE(TAG, "Failed to initialize NVS");
        improvLoop(display, improvManager,"NVS Init");
        return;
    }

    if (wifiManager.begin() != true) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi");
        improvLoop(display, improvManager, "Wi-Fi Init");
        vTaskDelay(pdMS_TO_TICKS(10000));
        return;
    }

    improvManager.begin();

    std::string ssid = wifiManager.loadSavedSsid();
    if (ssid.empty()) {
        ESP_LOGI(TAG, "No saved Wi-Fi credentials found");
        improvLoop(display, improvManager, "No Credentials");
    }

    ESP_LOGI(TAG, "Attempting to reconnect");
    showMessage(display, ssid.c_str());

    if (!wifiManager.connectToSaved()) {
        if (wifiManager.getStatus() == WifiManager::Status::NO_CREDENTIALS) {
            ESP_LOGI(TAG, "No Wi-Fi credentials found");
            improvLoop(display, improvManager, "No Credentials");
        }
        ESP_LOGI(TAG, "Connect Failed");
        improvLoop(display, improvManager, "Connect Failed");
    }



    PixelBounceAnimation anim(display, 4, 250, 125, 0, 50, 1000);
    BlockHeightFetcher fetcher;

    while (true) {
        display.clear();

        anim.update();
        fetcher.update();

        display.setColor(100, 100, 100);
        display.writeText(0, fetcher.getText(), true);
        display.render();

        improvManager.loop();
        
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}