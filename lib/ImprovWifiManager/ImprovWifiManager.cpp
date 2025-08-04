#include "ImprovWifiManager.hpp"
#include "NVSStore.hpp"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include <cstring>
#include <sstream>

#define MAX_ATTEMPTS_WIFI_CONNECTION 20

static const char* TAG = "ImprovWifiManager";

ImprovWifiManager* ImprovWifiManager::wifiManagerInstance = nullptr;

ImprovWifiManager::ImprovWifiManager(const std::vector<std::string>& deviceInfos) : deviceInfos(deviceInfos) {
    ESP_LOGI(TAG, "ImprovWifiManager constructor called");
    wifiManagerInstance = this;  // Set static instance pointer
}

ImprovWifiManager::~ImprovWifiManager() {
    ESP_LOGI(TAG, "ImprovWifiManager destructor called");
}

void ImprovWifiManager::begin() {
    ESP_LOGI(TAG, "ImprovWifiManager begin");

    connector.begin();

    // Init NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Init TCP/IP and event loop
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Optional: try connecting automatically to saved credentials
    connect_to_saved_wifi();
}

bool ImprovWifiManager::save_credentials(const std::string& ssid, const std::string& password) {
    NVSStore store("wifi_creds");
    bool success = store.save("ssid", ssid) && store.save("password", password);
    if (success) {
        ESP_LOGI(TAG, "Wi-Fi credentials saved.");
    } else {
        ESP_LOGE(TAG, "Failed to save Wi-Fi credentials.");
    }
    return success;
}

bool ImprovWifiManager::load_credentials(std::string& ssid, std::string& password) {
    NVSStore store("wifi_creds");
    bool success = store.load("ssid", ssid) && store.load("password", password);
    if (success) {
        ESP_LOGI(TAG, "Wi-Fi credentials loaded: SSID=%s", ssid.c_str());
    } else {
        ESP_LOGW(TAG, "Wi-Fi credentials not found.");
    }
    return success;
}

void ImprovWifiManager::connect_to_saved_wifi() {
    std::string ssid, password;
    if (!load_credentials(ssid, password)) {
        ESP_LOGW(TAG, "No credentials available to connect.");
        return;
    }

    wifi_config_t wifi_config = {};
    strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid), ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy(reinterpret_cast<char*>(wifi_config.sta.password), password.c_str(), sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    ESP_LOGI(TAG, "Attempting to connect to SSID: %s", ssid.c_str());
}

void ImprovWifiManager::loop() {
    connector.loop(static_onCommandCallback, onErrorCallback);
}

void ImprovWifiManager::onErrorCallback(improv::Error err) {
    ESP_LOGW(TAG, "Improv error occurred: %d", static_cast<int>(err));
}

bool ImprovWifiManager::static_onCommandCallback(improv::ImprovCommand cmd) {
    if (wifiManagerInstance != nullptr) {
        return wifiManagerInstance->onCommandCallback(cmd);
    }
    return false;
}

bool ImprovWifiManager::onCommandCallback(improv::ImprovCommand cmd) {
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

            if (connectWifi(cmd.ssid, cmd.password)) {
                save_credentials(cmd.ssid, cmd.password);
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

void ImprovWifiManager::getAvailableWifiNetworks() {
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

std::vector<std::string> ImprovWifiManager::getLocalUrl() {
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

bool ImprovWifiManager::connectWifi(const std::string& ssid, const std::string& password) {
    ESP_LOGI(TAG, "Attempting to connect to Wi-Fi SSID: %s", ssid.c_str());

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password));

    // Apply configuration and initiate connection
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    uint8_t attempt = 0;
    while (attempt <= MAX_ATTEMPTS_WIFI_CONNECTION) {
        wifi_ap_record_t ap_info;
        if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
            ESP_LOGI(TAG, "Wi-Fi connected to SSID: %s", reinterpret_cast<const char*>(ap_info.ssid));
            return true;
        }

        vTaskDelay(pdMS_TO_TICKS(500));  // Wait 500ms between checks
        attempt++;
    }

    ESP_LOGW(TAG, "Wi-Fi connection failed after %d attempts. Disconnecting...", attempt);
    esp_wifi_disconnect();
    return false;
}