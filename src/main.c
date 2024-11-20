/*
 * This file is part of DeskHop (https://github.com/hrvach/deskhop).
 * Copyright (c) 2024 Hrvoje Cavrak
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "main.h"

device_t global_state = {0};
device_t *state = &global_state;

void core1_main() {
  sleep_ms(10);

  uart_packet_t in_packet = {0};

  while (true) {
    // USB host task, needs to run as often as possible
    if (tuh_inited()) {
      tuh_task();
    }
    state->core1_last_loop_pass = time_us_64();
    receive_char(&in_packet, state);
  }
}

int main() {

  initial_setup(state);

  tud_init(BOARD_TUD_RHPORT);

  watchdog_enable(WATCHDOG_DELAY_MS, WATCHDOG_PAUSE_DEBUG);

  while (true) {
    kick_watchdog_task(state);
    // USB device task, needs to run as often as possible
    tud_task();

    screensaver_task(state);

    stdio_flush();
    sleep_us(10);
  }
}
