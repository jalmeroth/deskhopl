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

void enable_debug(void) {
  uart_send_value(ENABLE_DEBUG_MSG, 1);
  _enable_debug();
}

void _enable_debug(void) {
  stdio_uart_init_full(UART_ONE, UART_ONE_BAUD_RATE, UART_ONE_TX_PIN,
                       UART_ONE_RX_PIN);
}

/* This key combo locks both outputs simultaneously */
void lock_screen(void) {
  uart_send_value(LOCK_SCREEN_MSG, 1);
  send_lock_screen_report(NULL, NULL);
}

void request_reboot() {
  if (global_state.active_output == BOARD_ROLE) {
    global_state.reboot_requested = true;
  } else {
    uart_send_value(REQUEST_REBOOT_MSG, 1);
  }
}

void suspend_pc(void) {
  uart_send_value(SUSPEND_PC_MSG, 1);
  send_suspend_pc_report(NULL, NULL);
}

void screensaver_task(device_t *state) {
  const unsigned int mouse_move_delay = 1000000;
  uint64_t inactivity_period = time_us_64() - state->last_activity;

  static mouse_report_t report = {0};
  static int last_pointer_move = 0;
  static int jitter = 1;

  /* If we're not enabled, nothing to do here. */
  if (!SCREENSAVER_ENABLED)
    return;

  if (!state->tud_connected)
    return;

  /* System is still not idle for long enough to activate or we've been running
   * for too long */
  if (inactivity_period < SCREENSAVER_IDLE_TIME)
    return;

  /* We're active! Now check if it's time to move the cursor yet. */
  if ((time_us_32()) - last_pointer_move < mouse_move_delay)
    return;

  report.x = jitter;
  jitter = -jitter;

  /* Move mouse pointer */
  send_tud_report(ITF_NUM_HID_MS, REPORT_ID_MOUSE, 8, (uint8_t *)&report);

  /* Update timer of the last pointer move */
  last_pointer_move = time_us_32();
}

void send_lock_screen_report(uart_packet_t *packet, device_t *state) {
  (void)packet;
  (void)state;

  keyboard_report_t lock_report = {0}, release_keys = {0};
  uint8_t off, pos;

  switch (global_state.device_config[BOARD_ROLE].os) {
  case LINUX:
    lock_report.modifier = KEYBOARD_MODIFIER_LEFTGUI;
    off = get_byte_offset(HID_KEY_L);
    pos = get_pos_in_byte(HID_KEY_L);
    lock_report.keycode[off] = 1 << pos;
    break;
  case MACOS:
    lock_report.modifier =
        KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTGUI;
    off = get_byte_offset(HID_KEY_Q);
    pos = get_pos_in_byte(HID_KEY_Q);
    lock_report.keycode[off] = 1 << pos;
    break;
  }

  send_tud_report(ITF_NUM_HID_KB, REPORT_ID_KEYBOARD, sizeof(keyboard_report_t),
                  (uint8_t *)&lock_report);
  send_tud_report(ITF_NUM_HID_KB, REPORT_ID_KEYBOARD, sizeof(keyboard_report_t),
                  (uint8_t *)&release_keys);
}

void send_suspend_pc_report(uart_packet_t *packet, device_t *state) {
  (void)packet;
  (void)state;

  keyboard_report_t suspend_report = {0};
  uint8_t off, pos;

  switch (global_state.device_config[BOARD_ROLE].os) {
  case LINUX:
    suspend_report.modifier = KEYBOARD_MODIFIER_LEFTGUI |
                              KEYBOARD_MODIFIER_LEFTCTRL |
                              KEYBOARD_MODIFIER_LEFTSHIFT;
    off = get_byte_offset(HID_KEY_Q);
    pos = get_pos_in_byte(HID_KEY_Q);
    suspend_report.keycode[off] = 1 << pos;
    break;
  case MACOS:
    suspend_report.modifier =
        KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_LEFTGUI;
    break;
  }

  send_tud_report(ITF_NUM_HID_KB, REPORT_ID_KEYBOARD, sizeof(keyboard_report_t),
                  (uint8_t *)&suspend_report);

  if (global_state.device_config[BOARD_ROLE].os == MACOS) {
    consumer_report_t eject_report = {0};
    eject_report.apple = 1 << 3; // Usage (Eject)
    send_tud_report(ITF_NUM_HID_MS, 3, sizeof(consumer_report_t),
                    (uint8_t *)&eject_report);
    // we need to make sure MACOS that is not receiving
    // any reports until our USB device gets suspended
    switch_output_a(state);
  }
  set_tud_connected(false);
}

void set_onboard_led(device_t *state) {
  uint8_t new_led_state = (state->active_output == BOARD_ROLE);
  gpio_put(GPIO_LED_PIN, new_led_state);
}

void switch_output_a(device_t *state) {
  state->active_output = PICO_A;
  uart_send_value(OUTPUT_SELECT_MSG, state->active_output);
  set_onboard_led(state);
}

void toggle_output(void) {
  global_state.active_output ^= 1;
  uart_send_value(OUTPUT_SELECT_MSG, global_state.active_output);
  set_onboard_led(&global_state);
  release_all_keys();
}
