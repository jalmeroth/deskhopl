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

void convert_keycodes(const uint8_t *hid_report,
                      logitech_keyboard_report_t *new_report) {

  hid_keyboard_report_t *original = (hid_keyboard_report_t *)hid_report;
  new_report->modifier = original->modifier;

  for (uint8_t i = 0; i < 6; i++) {
    uint8_t key = original->keycode[i];
    if (key) {
      uint8_t off = get_byte_offset(key);
      uint8_t pos = get_pos_in_byte(key);
      new_report->keycode[off] ^= 1 << pos;
    }
  }
}

void handle_keyboard(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint8_t len) {

  hid_keyboard_report_t *original = (hid_keyboard_report_t *)report;
  logitech_keyboard_report_t pressed = {original->modifier, {0}};

  if (len == 8) {
    convert_keycodes(report, &pressed);
    report = (uint8_t *)&pressed;
    len = 16;
  }
  if (process_keyboard_report(report, len)) {
    send_x_report(KEYBOARD_REPORT_MSG, instance, REPORT_ID_KEYBOARD, report,
                  len);
  }
}

void handle_mouse(uint8_t instance, uint8_t report_id, uint8_t protocol,
                  uint8_t const *report, uint8_t len) {
  (void)protocol;
  if (report[0] == MOUSE_BUTTON_MIDDLE) {
    switch_output();
  } else {
    send_x_report(MOUSE_REPORT_MSG, instance, report_id, report, len);
  }
}

void handle_consumer(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint8_t len) {
  (void)protocol;
  send_x_report(CONSUMER_CONTROL_MSG, instance, report_id, report, len);
}

void handle_uart_generic_msg(uart_packet_t *packet, device_t *state) {
  (void)state;
  send_x_report(packet->type, packet->instance, packet->report_id, packet->data,
                packet->report_len);
}

void handle_uart_output_select_msg(uart_packet_t *packet, device_t *state) {
  state->active_output = packet->data[0];
  set_onboard_led();
}
