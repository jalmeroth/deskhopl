/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "main.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    // https://github.com/raspberrypi/usb-pid
    .idVendor = 0x2E8A,
    .idProduct = 0x107C,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
  return (uint8_t const *)&desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report_kb[] = {
    TUD_HID_REPORT_DESC_LOGI_KB(HID_REPORT_ID(REPORT_ID_KEYBOARD))};

uint8_t const desc_hid_report_ms[] = {
    TUD_HID_REPORT_DESC_LOGI_MS(HID_REPORT_ID(REPORT_ID_MOUSE))};

uint8_t const desc_hid_report_cd[] = {
    TUD_HID_REPORT_DESC_LOGI_CD(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL))};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
  switch (instance) {
  case 0:
    return desc_hid_report_kb;
  case 1:
    return desc_hid_report_ms;
  case 2:
    return desc_hid_report_cd;
  default:
    return NULL;
  }
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

#define CONFIG_TOTAL_LEN                                                       \
  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID_KB 0x81
#define EPNUM_HID_MS 0x82
#define EPNUM_HID_CD 0x83

uint8_t const desc_configuration[] = {
    // Config number, interface count, string index, total length, attribute,
    // power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 4, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500),

    // Interface number, string index(usb.iInterface), protocol, report
    // descriptor len, EP In
    // address, size & polling interval
    TUD_HID_DESCRIPTOR(ITF_NUM_HID_KB, 0, HID_ITF_PROTOCOL_KEYBOARD,
                       sizeof(desc_hid_report_kb), EPNUM_HID_KB,
                       CFG_TUD_HID_EP_BUFSIZE, 5),

    TUD_HID_DESCRIPTOR(ITF_NUM_HID_MS, 0, HID_ITF_PROTOCOL_MOUSE,
                       sizeof(desc_hid_report_ms), EPNUM_HID_MS,
                       CFG_TUD_HID_EP_BUFSIZE, 5),

    TUD_HID_DESCRIPTOR(ITF_NUM_HID_CD, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report_cd), EPNUM_HID_CD,
                       CFG_TUD_HID_EP_BUFSIZE, 5)};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
  (void)index; // for multiple configurations
  // This example use the same configuration for both high and full speed mode
  return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

// array of pointer to string descriptors
char const *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // 0: is supported language is English (0x0409)
    "Logitech",                 // 1: Manufacturer
    "USB Receiver",             // 2: Product
    NULL,                       // 3: Serials will use unique ID if possible
    "Mouse Helper"              // 4: Mouse Helper
};

static uint16_t _desc_str[32 + 1];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  (void)langid;
  (void)index;
  size_t chr_count;

  switch (index) {
  case STRID_LANGID:
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
    break;

    // case STRID_SERIAL:
    //   chr_count = board_usb_get_serial(_desc_str + 1, 32);
    //   break;

  default:
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (!(index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))
      return NULL;

    const char *str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    size_t const max_count =
        sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
    if (chr_count > max_count)
      chr_count = max_count;

    // Convert ASCII string into UTF-16
    for (size_t i = 0; i < chr_count; i++) {
      _desc_str[1 + i] = str[i];
    }
    break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}
