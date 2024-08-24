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

/**================================================== *
 * ==============  Checksum Functions  ============== *
 * ================================================== */

uint8_t calc_checksum(const uint8_t *data, int length) {
  uint8_t checksum = 0;

  for (int i = 0; i < length; i++) {
    checksum ^= data[i];
  }

  return checksum;
}

void set_tud_connected(bool connected) {
  global_state.tud_connected = connected;
  printf("tud connected: %s\r\n", connected ? "true" : "false");
}

bool verify_checksum(const uart_packet_t *packet) {
  uint8_t checksum = calc_checksum(packet->data, PACKET_DATA_LENGTH);
  return checksum == packet->checksum;
}
