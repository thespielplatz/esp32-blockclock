#include "ImprovConnector.hpp"
#include "improv.h"
#include "esp_log.h"
#include "driver/uart.h"

static const char* TAG = "ImprovConnector";

ImprovConnector::ImprovConnector() {
    ESP_LOGI(TAG, "ImprovConnector constructor called");
}

ImprovConnector::~ImprovConnector() {
    ESP_LOGI(TAG, "ImprovConnector destructor called");
}

void ImprovConnector::begin() {
    ESP_LOGI(TAG, "ImprovConnector begin");

    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(uart_num, &uart_config);
    uart_driver_install(uart_num, 1024, 0, 0, nullptr, 0);
    uart_set_pin(uart_num, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void ImprovConnector::set_state(improv::State state) {
    std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
    data.resize(11);
    data[6] = improv::IMPROV_SERIAL_VERSION;
    data[7] = improv::TYPE_CURRENT_STATE;
    data[8] = 1;
    data[9] = static_cast<uint8_t>(state);

    uint8_t checksum = 0x00;
    for (uint8_t d : data)
        checksum += d;
    data[10] = checksum;

    uart_write_bytes(uart_num, reinterpret_cast<const char*>(data.data()), data.size());
}

void ImprovConnector::set_error(improv::Error error) {
    std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
    data.resize(11);
    data[6] = improv::IMPROV_SERIAL_VERSION;
    data[7] = improv::TYPE_ERROR_STATE;
    data[8] = 1;
    data[9] = static_cast<uint8_t>(error);

    uint8_t checksum = 0x00;
    for (uint8_t d : data)
        checksum += d;
    data[10] = checksum;

    uart_write_bytes(uart_num, reinterpret_cast<const char*>(data.data()), data.size());
}

void ImprovConnector::send_response(std::vector<uint8_t> &response) {
    std::vector<uint8_t> data = {'I', 'M', 'P', 'R', 'O', 'V'};
    data.resize(9);
    data[6] = improv::IMPROV_SERIAL_VERSION;
    data[7] = improv::TYPE_RPC_RESPONSE;
    data[8] = static_cast<uint8_t>(response.size());
    data.insert(data.end(), response.begin(), response.end());

    uint8_t checksum = 0x00;
    for (uint8_t d : data)
        checksum += d;
    data.push_back(checksum);

    uart_write_bytes(uart_num, reinterpret_cast<const char*>(data.data()), data.size());
}

void ImprovConnector::loop(bool (*onCommandCallback)(improv::ImprovCommand), void (*onErrorCallback)(improv::Error)) {
    uint8_t b;

    while (uart_read_bytes(uart_num, &b, 1, pdMS_TO_TICKS(10)) > 0) {
        if (parse_improv_serial_byte(x_position, b, x_buffer, onCommandCallback, onErrorCallback)) {
            if (x_position < sizeof(x_buffer) - 1) {
                x_buffer[x_position++] = b;
            } else {
                ESP_LOGW(TAG, "Buffer overflow");
                x_position = 0;
            }
        } else {
            x_position = 0;
        }
    }
}
