#pragma once

#include <string>
#include "esp_err.h"

class NVSStore {
public:
    static bool initNvsFlash();
    explicit NVSStore(const char* namespace_name);
    ~NVSStore();

    bool save(const std::string& key, const std::string& value);
    bool load(const std::string& key, std::string& value);

private:
    std::string _namespace;
};
