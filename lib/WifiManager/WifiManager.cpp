#include "WifiManager.hpp"
#include "NVSStore.hpp"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <cstring>
#include <sstream>

WifiManager::WifiManager() {}

bool WifiManager::begin() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_netif_init() != ESP_OK ||
        esp_event_loop_create_default() != ESP_OK ||
        esp_wifi_init(&cfg) != ESP_OK ||
        esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK ||
        esp_wifi_start() != ESP_OK) {
        currentStatus = Status::ERROR;
        return false;
    }

    esp_netif_create_default_wifi_sta();
    currentStatus = Status::NOT_CONNECTED;
    return true;
}

bool WifiManager::connect(const std::string& ssid, const std::string& password) {
    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password));

    if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK ||
        esp_wifi_connect() != ESP_OK) {
        currentStatus = Status::ERROR;
        return false;
    }

    currentStatus = Status::CONNECTING;

    for (int i = 0; i < MAX_ATTEMPTS; ++i) {
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            currentStatus = Status::CONNECTED;
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    esp_wifi_disconnect();
    currentStatus = Status::NOT_CONNECTED;
    return false;
}

std::string WifiManager::loadSavedSsid() {
    std::string ssid, password;
    if (loadCredentials(ssid, password)) {
        return ssid;
    }
    return "";
}

bool WifiManager::connectToSaved() {
    std::string ssid, password;
    if (!loadCredentials(ssid, password)) {
        currentStatus = Status::NO_CREDENTIALS;
        return false;
    }
    return connect(ssid, password);
}

bool WifiManager::saveCredentials(const std::string& ssid, const std::string& password) {
    NVSStore store("wifi_creds");
    return store.save("ssid", ssid) && store.save("password", password);
}

bool WifiManager::loadCredentials(std::string& ssid, std::string& password) {
    NVSStore store("wifi_creds");
    return store.load("ssid", ssid) && store.load("password", password);
}

std::vector<std::tuple<std::string, int, bool>> WifiManager::scanNetworks() {
    std::vector<std::tuple<std::string, int, bool>> results;

    wifi_scan_config_t scan_config = {};
    scan_config.show_hidden = true;
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    uint16_t count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&count));
    wifi_ap_record_t* records = new wifi_ap_record_t[count];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&count, records));

    for (int i = 0; i < count; ++i) {
        std::string ssid((char*)records[i].ssid);
        int rssi = records[i].rssi;
        bool secured = records[i].authmode != WIFI_AUTH_OPEN;
        results.emplace_back(ssid, rssi, secured);
    }

    delete[] records;
    return results;
}

bool WifiManager::isConnected() {
    wifi_ap_record_t ap_info;
    return esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK;
}

WifiManager::Status WifiManager::getStatus() {
    return currentStatus;
}
