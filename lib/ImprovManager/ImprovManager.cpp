#include "ImprovManager.hpp"
#include "NVSStore.hpp"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <cstring>
#include <sstream>

#define MAX_ATTEMPTS_WIFI_CONNECTION 20

static const char* TAG = "ImprovManager";

ImprovManager* ImprovManager::managerInstance = nullptr;

ImprovManager::ImprovManager(const std::vector<std::string>& deviceInfos, WifiManager& wifiManager): wifiManager(wifiManager), deviceInfos(deviceInfos){
    managerInstance = this;  // Set static instance pointer
}

ImprovManager::~ImprovManager() {}

void ImprovManager::begin() {
    ESP_LOGI(TAG, "ImprovManager begin");
    connector.begin();
}

void ImprovManager::loop() {
    connector.loop(static_onCommandCallback, onErrorCallback);
}

void ImprovManager::onErrorCallback(improv::Error err) {
    ESP_LOGW(TAG, "Improv error occurred: %d", static_cast<int>(err));
}

bool ImprovManager::static_onCommandCallback(improv::ImprovCommand cmd) {
    if (managerInstance != nullptr) {
        return managerInstance->onCommandCallback(cmd);
    }
    return false;
}

bool ImprovManager::onCommandCallback(improv::ImprovCommand cmd) {
    ESP_LOGI(TAG, "Improv command received: %d", static_cast<int>(cmd.command));

    switch (cmd.command) {
        case improv::Command::GET_CURRENT_STATE: {
            wifi_ap_record_t ap_info;
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                connector.set_state(improv::State::STATE_PROVISIONED);
                std::vector<uint8_t> data = improv::build_rpc_response(improv::GET_CURRENT_STATE, getLocalUrl(), false);
                connector.send_response(data);
            } else {
                connector.set_state(improv::State::STATE_AUTHORIZED);
            }
            break;
        }

        case improv::Command::WIFI_SETTINGS: {
            if (cmd.ssid.empty()) {
                connector.set_error(improv::Error::ERROR_INVALID_RPC);
                break;
            }

            connector.set_state(improv::STATE_PROVISIONING);

            if (wifiManager.connect(cmd.ssid, cmd.password)) {
                wifiManager.saveCredentials(cmd.ssid, cmd.password);
                ESP_LOGI(TAG, "Wi-Fi credentials saved: SSID=%s", cmd.ssid.c_str());
                connector.set_state(improv::STATE_PROVISIONED);
                std::vector<uint8_t> data = improv::build_rpc_response(improv::WIFI_SETTINGS, getLocalUrl(), false);
                connector.send_response(data);
            } else {
                connector.set_state(improv::STATE_STOPPED);
                connector.set_error(improv::Error::ERROR_UNABLE_TO_CONNECT);
            }
            break;
        }

        case improv::Command::GET_DEVICE_INFO: {
            std::vector<uint8_t> data = improv::build_rpc_response(improv::GET_DEVICE_INFO, deviceInfos, false);
            connector.send_response(data);
            break;
        }

        case improv::Command::GET_WIFI_NETWORKS: {
            getAvailableWifiNetworks();
            break;
        }

        default:
            connector.set_error(improv::ERROR_UNKNOWN_RPC);
            return false;
    }

    return true;
}

void ImprovManager::getAvailableWifiNetworks() {
    ESP_LOGI(TAG, "Scanning for Wi-Fi networks...");

    // Configure scan settings
    wifi_scan_config_t scan_config = {};
    scan_config.ssid = NULL;
    scan_config.bssid = NULL;
    scan_config.channel = 0;
    scan_config.show_hidden = true;

    // Start scan (blocking = true)
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

    // Get scan results
    uint16_t ap_count = 0;
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

    wifi_ap_record_t *ap_records = new wifi_ap_record_t[ap_count];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));

    for (int i = 0; i < ap_count; ++i) {
        const wifi_ap_record_t& ap = ap_records[i];

        std::string ssid(reinterpret_cast<const char*>(ap.ssid));
        std::string rssi = std::to_string(ap.rssi);
        std::string secured = (ap.authmode == WIFI_AUTH_OPEN) ? "NO" : "YES";

        std::vector<uint8_t> data = improv::build_rpc_response(
            improv::GET_WIFI_NETWORKS, {ssid, rssi, secured}, false
        );
        connector.send_response(data);

        vTaskDelay(pdMS_TO_TICKS(1));  // Equivalent to Arduino's delay(1)
    }

    delete[] ap_records;

    // Final empty response to signal end of list
    std::vector<uint8_t> final_response = improv::build_rpc_response(
        improv::GET_WIFI_NETWORKS, std::vector<std::string>{}, false
    );
    connector.send_response(final_response);

    ESP_LOGI(TAG, "Wi-Fi scan complete, %d networks sent", ap_count);
}

std::vector<std::string> ImprovManager::getLocalUrl() {
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");

    if (netif == nullptr || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get local IP, returning empty URL");
        return {""};
    }

    // Format IP address
    std::stringstream ss;
    ss << "http://" << ((ip_info.ip.addr >> 0) & 0xFF) << "."
       << ((ip_info.ip.addr >> 8) & 0xFF) << "."
       << ((ip_info.ip.addr >> 16) & 0xFF) << "."
       << ((ip_info.ip.addr >> 24) & 0xFF);

    return { ss.str() };
}
