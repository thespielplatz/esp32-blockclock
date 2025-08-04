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

    // Improv callback handlers
    bool onCommandCallback(improv::ImprovCommand cmd);
    static void onErrorCallback(improv::Error err);
};
