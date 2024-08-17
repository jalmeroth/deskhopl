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

void switch_output(void) {
  global_state.active_output ^= 1;
  send_value(1, OUTPUT_SELECT_MSG);
}

void send_lock_screen_report(uart_packet_t *packet, device_t *state) {
  (void)packet;
  (void)state;

  logitech_keyboard_report_t lock_report = {0}, release_keys = {0};

  if (BOARD_ROLE == PICO_A) { // Linux
    lock_report.modifier = KEYBOARD_MODIFIER_LEFTGUI;
    uint8_t off = get_byte_offset(HID_KEY_L);
    uint8_t pos = get_pos_in_byte(HID_KEY_L);
    lock_report.keycode[off] = 1 << pos;
  } else if (BOARD_ROLE == PICO_B) { // MACOS
    lock_report.modifier =
        KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTGUI;
    uint8_t off = get_byte_offset(HID_KEY_Q);
    uint8_t pos = get_pos_in_byte(HID_KEY_Q);
    lock_report.keycode[off] = 1 << pos;
  }

  if (tud_ready()) {
    bool result = false;
    result = tud_hid_n_report(ITF_NUM_HID_KB, 0, (uint8_t *)&lock_report,
                              PACKET_DATA_LENGTH);
    printf("x[report] success: %s\r\n", result ? "true" : "false");
    result = tud_hid_n_report(ITF_NUM_HID_KB, 0, (uint8_t *)&release_keys,
                              PACKET_DATA_LENGTH);
    printf("x[report] success: %s\r\n", result ? "true" : "false");
  } else {
    printf("x[report] tud not ready\r\n");
  }
}

/* This key combo locks both outputs simultaneously */
void lock_screen(void) {
  send_lock_screen_report(NULL, NULL);
  send_value(1, LOCK_SCREEN_MSG);
}
