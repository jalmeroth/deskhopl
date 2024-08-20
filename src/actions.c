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

/* This key combo locks both outputs simultaneously */
void lock_screen(void) {
  send_lock_screen_report(NULL, NULL);
  send_value(1, LOCK_SCREEN_MSG);
}

void restore_leds(void) {
  // set_keyboard_leds();
  set_onboard_led();
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

  send_tud_report(ITF_NUM_HID_KB, REPORT_ID_KEYBOARD, (uint8_t *)&lock_report,
                  PACKET_DATA_LENGTH);
  send_tud_report(ITF_NUM_HID_KB, REPORT_ID_KEYBOARD, (uint8_t *)&release_keys,
                  PACKET_DATA_LENGTH);
}

void set_keyboard_leds(void) {
  if (BOARD_ROLE == PICO_A) {
    uint8_t leds;
    if (global_state.active_output == PICO_A) {
      leds = 1 << 1; // CapsLock
    } else {
      leds = 0;
    }
    tuh_hid_set_report(1, ITF_NUM_HID_KB, 0, HID_REPORT_TYPE_OUTPUT, &leds,
                       sizeof(uint8_t));
  }
}

void set_onboard_led(void) {
  uint8_t new_led_state = (global_state.active_output == BOARD_ROLE);
  gpio_put(GPIO_LED_PIN, new_led_state);
}

void switch_output(void) {
  global_state.active_output ^= 1;
  send_value(global_state.active_output, OUTPUT_SELECT_MSG);
  restore_leds();
}
