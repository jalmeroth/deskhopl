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

hotkey_combo_t hotkeys[] = {
    /* Main keyboard switching hotkey */
    {.modifier = 0,
     .keys = {HID_KEY_CAPS_LOCK},
     .key_count = 1,
     .pass_to_os = false,
     .action_handler = &switch_output},
    {.modifier = KEYBOARD_MODIFIER_RIGHTALT | KEYBOARD_MODIFIER_RIGHTSHIFT,
     .keys = {HID_KEY_L},
     .key_count = 1,
     .pass_to_os = false,
     .action_handler = &lock_screen},
};
