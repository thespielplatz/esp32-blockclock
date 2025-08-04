#pragma once

#include <string>
#include "ImprovConnector.hpp"
#include "WifiManager.hpp"

class ImprovManager {
public:
    ImprovManager(const std::vector<std::string>& deviceInfos, WifiManager& wifiManager);
    ~ImprovManager();

    void begin();

    void getAvailableWifiNetworks();

    std::vector<std::string> getLocalUrl();

    void loop();

private:
    static bool static_onCommandCallback(improv::ImprovCommand cmd);
    static ImprovManager* managerInstance;  // Static pointer to delegate to

    ImprovConnector connector;
    WifiManager& wifiManager;
    std::vector<std::string> deviceInfos;

    // Improv callback handlers
    bool onCommandCallback(improv::ImprovCommand cmd);
    static void onErrorCallback(improv::Error err);
};
