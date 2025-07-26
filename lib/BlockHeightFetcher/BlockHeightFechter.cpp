#include "BlockHeightFetcher.hpp"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_crt_bundle.h"

BlockHeightFetcher::BlockHeightFetcher(uint32_t okIntervalMs, uint32_t errRetryMs, uint32_t failThresholdMs)
    : okInterval(okIntervalMs), errRetry(errRetryMs), failThreshold(failThresholdMs)
{
    lastSuccessUs = esp_timer_get_time();
    lastAttemptUs = 0;
}

void BlockHeightFetcher::update() {
    uint64_t nowUs = esp_timer_get_time();
    uint64_t msSinceAttempt = (nowUs - lastAttemptUs) / 1000;
    uint64_t msSinceSuccess = (nowUs - lastSuccessUs) / 1000;

    bool shouldFetch = false;

    if (errorMode && msSinceAttempt >= errRetry) {
        shouldFetch = true;
    } else if (!errorMode && msSinceAttempt >= okInterval) {
        shouldFetch = true;
    }

    if (shouldFetch) {
        lastAttemptUs = nowUs;
        int height = fetch_block_height();
        if (height > 0) {
            text = std::to_string(height);
            lastSuccessUs = nowUs;
            errorMode = false;
        } else if (msSinceSuccess >= failThreshold) {
            text = "REQ ERR";
            errorMode = true;
        }
    }
}

std::string BlockHeightFetcher::getText() const {
    return text;
}

int BlockHeightFetcher::fetch_block_height() {
    const char* TAG = "HTTPS";
    esp_http_client_config_t config = {
        .url = "https://mempool.space/api/blocks/tip/height",
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return -1;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "HTTP GET failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return -1;
    }

    char buffer[16];
    int len = esp_http_client_read(client, buffer, sizeof(buffer) - 1);
    buffer[len] = '\0';
    esp_http_client_cleanup(client);

    ESP_LOGI(TAG, "Block height: %s", buffer);
    return atoi(buffer);
}
