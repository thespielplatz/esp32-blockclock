#pragma once

#include <vector>
#include "improv.h"
#include "driver/uart.h"

class ImprovConnector {
public:
    ImprovConnector();
    ~ImprovConnector();

    void begin();

    void set_state(improv::State state);
    void set_error(improv::Error error);
    void send_response(std::vector<uint8_t> &response);
    void loop(bool (*onCommandCallback)(improv::ImprovCommand), void (*onErrorCallback)(improv::Error));

private:
    static constexpr uart_port_t uart_num = UART_NUM_0; // adjust if needed

    static constexpr int BUF_SIZE = 256;
    uint8_t x_buffer[BUF_SIZE];   // Buffer for incoming Improv serial data
    size_t x_position = 0;        // Current position in buffer
};
