#pragma once
#include <string>
#include <vector>
#include "improv.h"
#include "WifiManager.hpp"
#include "ImprovConnector.hpp"

class ImprovConnectorHandler {
public:
    ImprovConnectorHandler(ImprovConnector& connector, WifiManager& wifi, const std::vector<std::string>& deviceInfos);
    void loop();
    bool handleCommand(const improv::ImprovCommand& cmd);

private:
    void onError(improv::Error err);
    std::vector<std::string> getLocalUrl() const;
    void respondWithNetworks();

    ImprovConnector& connector;
    WifiManager& wifi;
    std::vector<std::string> deviceInfos;
    uint8_t buffer[256];
    size_t position = 0;
};
