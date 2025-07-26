#include "BlockHeightFetcher.hpp"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_crt_bundle.h"

#define MAX_HTTP_OUTPUT_BUFFER 64  // Plenty for the block height string

BlockHeightFetcher::BlockHeightFetcher(uint32_t okIntervalMs, uint32_t errRetryMs, uint32_t failThresholdMs)
    : okInterval(okIntervalMs), errRetry(errRetryMs), failThreshold(failThresholdMs)
{
    lastSuccessUs = 0;
    lastAttemptUs = 0;
}

void BlockHeightFetcher::update() {
    uint64_t nowUs = esp_timer_get_time();
    uint64_t msSinceAttempt = (nowUs - lastAttemptUs) / 1000;
    uint64_t msSinceSuccess = (nowUs - lastSuccessUs) / 1000;

    bool shouldFetch = false;

    if (firstFetch) {
        firstFetch = false;
        shouldFetch = true;
    } else if (errorMode && msSinceAttempt >= errRetry) {
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
    int content_length = 0;

    // Response buffer, one extra byte for null-termination safety
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};

    esp_http_client_config_t config = {
        .url = "https://mempool.space/api/blocks/tip/height",
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    ESP_LOGI(TAG, "HTTP GET request => %s", config.url);

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return -1;
    }

    // Set to GET (explicitly)
    esp_http_client_set_method(client, HTTP_METHOD_GET);

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return -1;
    }

    content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        ESP_LOGE(TAG, "Failed to fetch headers");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return -1;
    }

    int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
    if (data_read >= 0) {
        output_buffer[data_read] = '\0';  // Ensure null-termination

        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
            esp_http_client_get_status_code(client),
            content_length);

        ESP_LOGI(TAG, "Block height: %s", output_buffer);
    } else {
        ESP_LOGE(TAG, "Failed to read response");
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
        return -1;
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return atoi(output_buffer);  // Convert plain number to int
}
