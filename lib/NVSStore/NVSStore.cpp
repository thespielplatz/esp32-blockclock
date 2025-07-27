#include "NVSStore.hpp"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char* TAG = "NVSStore";

NVSStore::NVSStore(const char* namespace_name) : _namespace(namespace_name) {}

NVSStore::~NVSStore() {}

bool NVSStore::save(const std::string& key, const std::string& value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(_namespace.c_str(), NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }

    err = nvs_set_str(handle, key.c_str(), value.c_str());
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set key: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    nvs_commit(handle);
    nvs_close(handle);
    ESP_LOGI(TAG, "Saved key '%s' to namespace '%s'", key.c_str(), _namespace.c_str());
    return true;
}

bool NVSStore::load(const std::string& key, std::string& value) {
    nvs_handle_t handle;
    esp_err_t err = nvs_open(_namespace.c_str(), NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to open NVS for reading: %s", esp_err_to_name(err));
        return false;
    }

    size_t required_size = 0;
    err = nvs_get_str(handle, key.c_str(), nullptr, &required_size);
    if (err != ESP_OK || required_size == 0) {
        ESP_LOGW(TAG, "Key '%s' not found or empty", key.c_str());
        nvs_close(handle);
        return false;
    }

    char* buffer = new char[required_size];
    err = nvs_get_str(handle, key.c_str(), buffer, &required_size);
    if (err == ESP_OK) {
        value.assign(buffer);
    } else {
        ESP_LOGE(TAG, "Failed to load key '%s': %s", key.c_str(), esp_err_to_name(err));
    }
    delete[] buffer;
    nvs_close(handle);
    return (err == ESP_OK);
}
