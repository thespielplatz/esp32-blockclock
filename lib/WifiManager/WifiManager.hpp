#pragma once
#include <string>
#include <vector>
#include <tuple>

class WifiManager {
public:
    enum class Status {
        NO_CREDENTIALS,
        CONNECTING,
        CONNECTED,
        NOT_CONNECTED,
        ERROR
    };

    WifiManager();
    bool begin();
    bool connect(const std::string& ssid, const std::string& password);
    bool connectToSaved();
    bool saveCredentials(const std::string& ssid, const std::string& password);
    bool loadCredentials(std::string& ssid, std::string& password);
    std::vector<std::tuple<std::string, int, bool>> scanNetworks();
    bool isConnected();
    Status getStatus();
    std::string loadSavedSsid();

private:
    static constexpr uint8_t MAX_ATTEMPTS = 20;
    Status currentStatus = Status::NOT_CONNECTED;
};