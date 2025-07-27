#pragma once

#include <string>
#include "ImprovConnector.hpp"

class ImprovWifiManager {
public:
    ImprovWifiManager(const std::vector<std::string>& deviceInfos);
    ~ImprovWifiManager();

    void begin();

    bool save_credentials(const std::string& ssid, const std::string& password);
    bool load_credentials(std::string& ssid, std::string& password);
    void connect_to_saved_wifi();
    void getAvailableWifiNetworks();
    bool connectWifi(const std::string& ssid, const std::string& password);

    std::vector<std::string> getLocalUrl();

    void loop();

private:
    static bool static_onCommandCallback(improv::ImprovCommand cmd);
    static ImprovWifiManager* wifiManagerInstance;  // Static pointer to delegate to

    ImprovConnector connector;
    std::vector<std::string> deviceInfos;

    static constexpr uart_port_t uart_num = UART_NUM_0;
    static constexpr int BUF_SIZE = 256;

    uint8_t x_buffer[BUF_SIZE];   // Buffer for incoming Improv serial data
    size_t x_position = 0;        // Current position in buffer

    // Improv callback handlers
    bool onCommandCallback(improv::ImprovCommand cmd);
    static void onErrorCallback(improv::Error err);
};
