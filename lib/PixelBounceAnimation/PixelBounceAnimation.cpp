#include "PixelBounceAnimation.hpp"
#include "esp_timer.h"

PixelBounceAnimation::PixelBounceAnimation(Display& disp,
                                           uint16_t row,
                                           uint8_t r, uint8_t g, uint8_t b,
                                           uint16_t speed_ms,
                                           uint16_t wait_ms)
    : display(disp),
      row(row),
      speed_ms(speed_ms),
      wait_ms(wait_ms),
      state(State::Forward),
      position(disp.getWidth() - 1),
      direction(-1),
      last_update(esp_timer_get_time() / 1000),
      wait_start(0)
{
    color[0] = r;
    color[1] = g;
    color[2] = b;
}

void PixelBounceAnimation::reset() {
    position = display.getWidth() - 1;
    direction = -1;
    state = State::Forward;
    last_update = esp_timer_get_time() / 1000;
}

void PixelBounceAnimation::update() {
    drawFrame();
    uint32_t now = esp_timer_get_time() / 1000;
    if ((now - last_update) < speed_ms) return;

    last_update = now;


    int width = display.getWidth();

    if (state == State::Forward || state == State::Backward) {
        position += direction;

        if (position <= 0 || position >= width - 1) {
            position = std::max(0, std::min(position, width - 1));
            state = State::Waiting;
            wait_start = now;
            direction *= -1;
        }
    } else if (state == State::Waiting) {
        if (now - wait_start >= wait_ms) {
            state = direction == -1 ? State::Forward : State::Backward;
        }
    }
}

void PixelBounceAnimation::drawFrame() {
    const int trail = 10;
    for (int i = 0; i < trail; ++i) {
        int x = position - direction * i;
        if (x < 0 || x >= display.getWidth()) continue;

        float factor = 1.0f - float(i) / float(trail);
        display.set_pixel(x, row,
            color[0] * factor,
            color[1] * factor,
            color[2] * factor);
    }
}
