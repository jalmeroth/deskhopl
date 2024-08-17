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

#define MAX_REPORT 4

// Each HID instance can have multiple reports
static struct {
  uint8_t report_count;
  tuh_hid_report_info_t report_info[MAX_REPORT];
} hid_info[CFG_TUH_HID];

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const *report, uint16_t len) {
  // lets dertermine protocol mode first
  uint8_t protocol = tuh_hid_get_protocol(dev_addr, instance);
  printf("h[report] dev_addr: %d instance: %d protocol: %d\r\n", dev_addr,
         instance, protocol);

  uint8_t report_id = 0, usage_page = 0, usage = 0;

  if (protocol == HID_PROTOCOL_REPORT) {
    // do we have more then one report
    if (hid_info[instance].report_count > 1) {
      // we should have a report_id for this, right?
      report_id = report[0];
      report++;
      len--;
    }
    // dertermine usage_page/usage from parsed reports
    for (uint8_t i = 0; i < hid_info[instance].report_count; i++) {
      if (report_id == hid_info[instance].report_info[i].report_id) {
        usage_page = hid_info[instance].report_info[i].usage_page;
        usage = hid_info[instance].report_info[i].usage;
        break;
      }
    }
  } else { // HID_PROTOCOL_BOOT
    usage_page = hid_info[instance].report_info[0].usage_page;
    usage = hid_info[instance].report_info[0].usage;
  }

  printf("h[report]: %d, usage_page: %#06x, usage: %#04x, len: %d\r\n",
         report_id, usage_page, usage, len);

  if (usage_page == HID_USAGE_PAGE_DESKTOP) {
    if (usage == HID_USAGE_DESKTOP_KEYBOARD) {
      handle_keyboard(instance, report_id, protocol, report, len);
    } else if (usage == HID_USAGE_DESKTOP_MOUSE) {
      handle_mouse(instance, report_id, protocol, report, len);
    }
  } else if (usage_page == HID_USAGE_PAGE_CONSUMER) {
    if (usage == HID_USAGE_CONSUMER_CONTROL) {
      handle_consumer(instance, report_id, protocol, report, len);
    }
  }
  tuh_hid_receive_report(dev_addr, instance);
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
  printf("h[umount] dev_addr: %d, instance: %d\r\n", dev_addr, instance);
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const *desc_report, uint16_t desc_len) {
  printf("h[mount] dev_addr: %d, instance: %d, len: %d\r\n", dev_addr, instance,
         desc_len);

  // Interface protocol (hid_interface_protocol_enum_t)
  const char *protocol_str[] = {"None", "Keyboard", "Mouse"};
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  printf("HID Interface Protocol = %s\r\n", protocol_str[itf_protocol]);

  // By default host stack will use activate boot protocol on supported
  // interface. Therefore for this simple example, we only need to parse generic
  // report descriptor (with built-in parser)
  hid_info[instance].report_count = tuh_hid_parse_report_descriptor(
      hid_info[instance].report_info, MAX_REPORT, desc_report, desc_len);
  printf("HID has %u reports \r\n", hid_info[instance].report_count);

  if (hid_info[instance].report_count > 1) {
    for (uint8_t i = 0; i < hid_info[instance].report_count; i++) {
      printf("report: %d, usage: %#04x, usage_page: %#06x\r\n",
             hid_info[instance].report_info[i].report_id,
             hid_info[instance].report_info[i].usage,
             hid_info[instance].report_info[i].usage_page);
    }
  }

  // request to receive report
  // tuh_hid_report_received_cb() will be invoked when report is available
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    printf("Error: cannot request to receive report\r\n");
  }
}
