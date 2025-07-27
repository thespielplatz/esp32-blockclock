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

private:
    static constexpr uart_port_t uart_num = UART_NUM_0; // adjust if needed
};
