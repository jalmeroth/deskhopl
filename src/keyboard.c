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

#include "hotkeys.h"
#include "main.h"

uint8_t get_byte_offset(uint8_t key) {
  uint8_t offset = (key - HID_KEY_A) / 8;
  return offset;
}

uint8_t get_pos_in_byte(uint8_t key) {
  uint8_t pos = (key - HID_KEY_A) % 8;
  return pos;
}

/* Tries to find if the keyboard report contains key, returns true/false */
bool key_in_report(uint8_t key, const keyboard_report_t *report) {
  uint8_t off = get_byte_offset(key);
  uint8_t pos = get_pos_in_byte(key);
  uint8_t val = 1 << pos;

  if (report->keycode[off] == val) {
    return true;
  }
  return false;
}

/* Check if the current report matches a specific hotkey passed on */
bool check_specific_hotkey(hotkey_combo_t keypress,
                           const keyboard_report_t *report) {
  /* We expect all modifiers specified to be detected in the report */
  if (keypress.modifier != (report->modifier & keypress.modifier))
    return false;

  for (int n = 0; n < keypress.key_count; n++) {
    if (!key_in_report(keypress.keys[n], report)) {
      return false;
    }
  }

  /* Getting here means all of the keys were found. */
  return true;
}

/* Go through the list of hotkeys, check if any of them match. */
hotkey_combo_t *check_all_hotkeys(keyboard_report_t *report) {
  for (int n = 0; n < ARRAY_SIZE(hotkeys); n++) {
    if (check_specific_hotkey(hotkeys[n], report)) {
      return &hotkeys[n];
    }
  }
  return NULL;
}

bool process_keyboard_report(uint8_t const *report, uint8_t len) {
  keyboard_report_t *keyboard_report = (keyboard_report_t *)report;
  hotkey_combo_t *hotkey = NULL;
  bool pass_to_os = true;

  hotkey = check_all_hotkeys(keyboard_report);

  /* ... and take appropriate action */
  if (hotkey != NULL) {
    /* Execute the corresponding handler */
    hotkey->action_handler(keyboard_report);

    /* And pass the key to the output PC if configured to do so. */
    pass_to_os = hotkey->pass_to_os;
  }

  return pass_to_os;
}

bool release_all_keys(void) {
  // release keys if any were pressed
  keyboard_report_t release_keys = {0};
  return send_tud_report(ITF_NUM_HID_KB, REPORT_ID_KEYBOARD,
                         sizeof(keyboard_report_t), (uint8_t *)&release_keys);
}
