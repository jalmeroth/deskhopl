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

void handle_keyboard(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint16_t len) {
  if (protocol == HID_PROTOCOL_REPORT) {
    if (process_keyboard_report(report, len)) {
      send_n_report(KEYBOARD_REPORT_MSG, instance, REPORT_ID_KEYBOARD, report,
                    len);
    }
  }
}

void handle_mouse(uint8_t instance, uint8_t report_id, uint8_t protocol,
                  uint8_t const *report, uint16_t len) {
  (void)protocol;
  if (report[0] == MOUSE_BUTTON_MIDDLE) {
    switch_output();
  } else {
    send_n_report(MOUSE_REPORT_MSG, instance, report_id, report, len);
  }
}

void handle_consumer(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint16_t len) {
  (void)protocol;
  send_n_report(CONSUMER_CONTROL_MSG, instance, report_id, report, len);
}

void handle_generic_uart_msg(uart_packet_t *packet, device_t *state) {
  (void)state;
  send_n_report(packet->type, packet->instance, packet->report_id, packet->data,
                packet->report_len);
}

void handle_output_select_uart_msg(uart_packet_t *packet, device_t *state) {
  state->active_output ^= 1;
  tud_remote_wakeup();
}

bool send_n_report(enum packet_type_e packet_type, uint8_t instance,
                   uint8_t report_id, uint8_t const *report, uint16_t len) {
  bool result = false;

  if (len > PACKET_DATA_LENGTH) {
    // stop flooding on disconnects?
    return result;
  }

  printf("x[report] instance: %d report_id: %d size %d\r\n", instance,
         report_id, len);
  for (uint32_t i = 0; i < len; i++) {
    printf("%02x ", report[i]);
  }
  printf("\r\n");

  if (BOARD_ROLE == global_state.active_output) {
    if (tud_ready()) {
      result = tud_hid_n_report(instance, report_id, report, len);
      printf("x[report] success: %s\r\n", result ? "true" : "false");
    } else {
      printf("x[report] tud not ready\r\n");
    }
  } else {
    send_packet(instance, report_id, (uint8_t *)report, packet_type, len);
  }

  return result;
}
