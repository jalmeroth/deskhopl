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

// Invoked when device is mounted
void tud_mount_cb(void) {
  printf("d[mount]\n");
  set_tud_connected(true);
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
  printf("d[umount]\n");
  set_tud_connected(false);
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
  // (void)remote_wakeup_en;
  printf("d[suspend] wakeup: %s\n", remote_wakeup_en ? "true" : "false");
  set_tud_connected(false);
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
  printf("d[resume]\n");
  // if (global_state.device_config[BOARD_ROLE].os == MACOS) {
  //   tud_deinit(BOARD_TUD_RHPORT);
  //   tud_init(BOARD_TUD_RHPORT);
  // }
  set_tud_connected(true);
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report,
                                uint16_t len) {
  // printf("d[report-complete] instance: %d\r\n", instance);
  (void)report;
  (void)len;
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
  printf("d[2] instance: %d\r\n", instance);
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  (void)report_id;
  (void)bufsize;
  // mostly we get type out reports from host to update keyboard leds.
  // we ignore these reports for now.
  printf("d[set_report] idx: %x, type: %x, buf: %x\r\n", instance, report_type,
         buffer[0]);
}
