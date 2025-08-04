#include "ImprovConnectorHandler.hpp"
#include "esp_netif.h"
#include "esp_log.h"
#include <sstream>

ImprovConnectorHandler::ImprovConnectorHandler(ImprovConnector& conn, WifiManager& wifiMgr, const std::vector<std::string>& infos)
    : connector(conn), wifi(wifiMgr), deviceInfos(infos) {}

void ImprovConnectorHandler::loop() {
    uint8_t b;
    while (uart_read_bytes(UART_NUM_0, &b, 1, pdMS_TO_TICKS(10)) > 0) {
        if (parse_improv_serial_byte(position, b, buffer, nullptr, nullptr)) {
            if (position < sizeof(buffer) - 1) {
                buffer[position++] = b;
            } else {
                ESP_LOGW("ImprovHandler", "Buffer overflow");
                position = 0;
            }
        } else {
            position = 0;
        }
    }
}

bool ImprovConnectorHandler::handleCommand(const improv::ImprovCommand& cmd) {
    switch (cmd.command) {
        case improv::Command::GET_CURRENT_STATE: {
            wifi_ap_record_t ap_info;
            if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                connector.set_state(improv::STATE_PROVISIONED);
                connector.send_response(improv::build_rpc_response(cmd.command, getLocalUrl(), false));
            } else {
                connector.set_state(improv::STATE_AUTHORIZED);
            }
            break;
        }
        case improv::Command::WIFI_SETTINGS: {
            if (cmd.ssid.empty()) {
                connector.set_error(improv::ERROR_INVALID_RPC);
                break;
            }
            connector.set_state(improv::STATE_PROVISIONING);
            if (wifi.connect(cmd.ssid, cmd.password)) {
                wifi.saveCredentials(cmd.ssid, cmd.password);
                connector.set_state(improv::STATE_PROVISIONED);
                connector.send_response(improv::build_rpc_response(cmd.command, getLocalUrl(), false));
            } else {
                connector.set_state(improv::STATE_STOPPED);
                connector.set_error(improv::ERROR_UNABLE_TO_CONNECT);
            }
            break;
        }
        case improv::Command::GET_DEVICE_INFO: {
            connector.send_response(improv::build_rpc_response(cmd.command, deviceInfos, false));
            break;
        }
        case improv::Command::GET_WIFI_NETWORKS: {
            respondWithNetworks();
            break;
        }
        default:
            connector.set_error(improv::ERROR_UNKNOWN_RPC);
            return false;
    }
    return true;
}

void ImprovConnectorHandler::respondWithNetworks() {
    auto results = wifi.scanNetworks();
    for (const auto& [ssid, rssi, secured] : results) {
        connector.send_response(improv::build_rpc_response(
            improv::GET_WIFI_NETWORKS, {ssid, std::to_string(rssi), secured ? "YES" : "NO"}, false));
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    connector.send_response(improv::build_rpc_response(improv::GET_WIFI_NETWORKS, {}, false));
}

std::vector<std::string> ImprovConnectorHandler::getLocalUrl() const {
    esp_netif_ip_info_t ip_info;
    esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!netif || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) return {""};

    std::stringstream ss;
    ss << "http://" << ((ip_info.ip.addr >> 0) & 0xFF) << "."
       << ((ip_info.ip.addr >> 8) & 0xFF) << "."
       << ((ip_info.ip.addr >> 16) & 0xFF) << "."
       << ((ip_info.ip.addr >> 24) & 0xFF);
    return { ss.str() };
}
