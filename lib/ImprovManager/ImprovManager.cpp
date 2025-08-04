class ImprovManager {
public:
    ImprovManager(const std::vector<std::string>& deviceInfos);
    void begin();
    void loop();

private:
    WifiManager wifi;
    ImprovConnector connector;
    ImprovConnectorHandler improv;
};
