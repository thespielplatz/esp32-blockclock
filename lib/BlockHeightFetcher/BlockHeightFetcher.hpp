#pragma once

#include <string>
#include <stdint.h>

class BlockHeightFetcher {
public:
    BlockHeightFetcher(uint32_t okIntervalMs = 60000, uint32_t errRetryMs = 2000, uint32_t failThresholdMs = 300000);
    
    void update();            // Call regularly to handle timed fetch
    std::string getText() const; // Returns current text (block height or "REQ ERR")

private:
    std::string text = "FETCHING";
    bool firstFetch = true; // Flag to indicate if this is the first fetch
    bool errorMode = false;

    uint64_t lastSuccessUs = 0;
    uint64_t lastAttemptUs = 0;

    uint32_t okInterval;
    uint32_t errRetry;
    uint32_t failThreshold;

    int fetch_block_height(); // internal fetch call
};
