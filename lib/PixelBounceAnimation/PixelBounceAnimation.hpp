#pragma once
#include "Display.hpp"
#include <stdint.h>

class PixelBounceAnimation {
public:
    PixelBounceAnimation(Display& display,
                         uint16_t row,
                         uint8_t r, uint8_t g, uint8_t b,
                         uint16_t speed_ms = 50,
                         uint16_t wait_ms = 500);

    void update();      // Call this regularly (e.g. from a loop)
    void reset();       // Optional reset

private:
    enum class State { Forward, Backward, Waiting };
    
    Display& display;
    uint16_t row;
    uint8_t color[3];
    uint16_t speed_ms;
    uint16_t wait_ms;

    State state;
    int position;
    int8_t direction;
    uint32_t last_update;
    uint32_t wait_start;

    void drawFrame();
};
