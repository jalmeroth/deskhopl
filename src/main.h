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

#pragma once
// INCLUDES
#include "hardware/clocks.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pio_usb.h"
#include "tusb.h"
#include "tusb_descriptors.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BOARD CONFIG
#define PICO_A 0
#define PICO_B 1
#define GPIO_LED_PIN 25 // LED is connected to pin 25 on a PICO

// UART CONFIG
#define UART_ZERO uart0
#define UART_ONE uart1
#define UART_ZERO_BAUD_RATE 3686400
#define UART_ONE_BAUD_RATE 115200
#define UART_ONE_TX_PIN 4
#define UART_ONE_RX_PIN 5
#if BOARD_ROLE == PICO_A
#define BOARD_NAME "PICO_A"
#define UART_TX_PIN 12
#define UART_RX_PIN 13
#elif BOARD_ROLE == PICO_B
#define BOARD_NAME "PICO_B"
#define UART_TX_PIN 16
#define UART_RX_PIN 17
#endif
/*********  Protocol definitions  *********
 *
 * - every packet starts with 0xAA 0x55 for easy re-sync
 * - then a 1 byte packet type is transmitted
 * - 8 bytes of packet data follows, fixed length for simplicity
 * - 1 checksum byte ends the packet
 *      - checksum includes **only** the packet data
 *      - checksum is simply calculated by XORing all bytes together
 */

enum packet_type_e {
  KEYBOARD_REPORT_MSG = 1,
  MOUSE_REPORT_MSG = 2,
  OUTPUT_SELECT_MSG = 3,
  FIRMWARE_UPGRADE_MSG = 4,
  MOUSE_ZOOM_MSG = 5,
  KBD_SET_REPORT_MSG = 6,
  SWITCH_LOCK_MSG = 7,
  SYNC_BORDERS_MSG = 8,
  FLASH_LED_MSG = 9,
  SCREENSAVER_MSG = 10,
  WIPE_CONFIG_MSG = 11,
  SWAP_OUTPUTS_MSG = 12,
  HEARTBEAT_MSG = 13,
  OUTPUT_CONFIG_MSG = 14,
  CONSUMER_CONTROL_MSG = 15,
  LOCK_SCREEN_MSG = 16,
};
typedef enum { IDLE, READING_PACKET, PROCESSING_PACKET } receiver_state_t;

typedef struct {
  uint8_t active_output; // Currently selected output (0 = A, 1 = B)
  receiver_state_t
      receiver_state; // Storing the state for the simple receiver state machine
} device_t;

typedef void (*action_handler_t)();

typedef struct { // Maps message type -> message handler function
  enum packet_type_e type;
  action_handler_t handler;
} uart_handler_t;

// Logitech HID Report Protocol Keyboard Report.
typedef struct TU_ATTR_PACKED {
  uint8_t modifier;    /**< Keyboard modifier (KEYBOARD_MODIFIER_* masks). */
  uint8_t keycode[15]; /**< Key codes of the currently pressed keys. */
} logitech_keyboard_report_t;

typedef struct {
  uint8_t modifier;  // Which modifier is pressed
  uint8_t keys[14];  // Which keys need to be pressed
  uint8_t key_count; // How many keys are pressed
  action_handler_t
      action_handler; // What to execute when the key combination is detected
  bool pass_to_os;    // True if we are to pass the key to the OS too
  bool acknowledge;   // True if we are to notify the user about registering
                      // keypress
} hotkey_combo_t;

/*********  Packet parameters  **********/

#define START1 0xAA
#define START2 0x55
#define START_LENGTH 2

#define TYPE_LENGTH 1
#define INSTANCE_LENGTH 1
#define REPORT_ID_LENGTH 1
#define REPORT_LEN_LENGTH 1
// For simplicity, all packet types are the same length
#define PACKET_DATA_LENGTH 16
#define CHECKSUM_LENGTH 1

#define PACKET_LENGTH                                                          \
  (TYPE_LENGTH + INSTANCE_LENGTH + REPORT_ID_LENGTH + REPORT_LEN_LENGTH +      \
   PACKET_DATA_LENGTH + CHECKSUM_LENGTH)
#define RAW_PACKET_LENGTH (START_LENGTH + PACKET_LENGTH)

/* Data structure defining packets of information transferred */
typedef struct {
  uint8_t type;                     // Enum field describing the type of packet
  uint8_t instance;                 // instance
  uint8_t report_id;                // report_id
  uint8_t report_len;               // report_len
  uint8_t data[PACKET_DATA_LENGTH]; // Data goes here
  uint8_t checksum;                 // Checksum, a simple XOR-based one
} uart_packet_t;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
// setup.c
void core1_main(void);
void initial_setup(void);
void setup_uart(void);
void setup_tuh(void);
// actions.c
void lock_screen(void);
void restore_leds(void);
void send_lock_screen_report(uart_packet_t *packet, device_t *state);
void set_keyboard_leds(void);
void set_onboard_led(void);
void switch_output(void);
// handlers.c
void handle_keyboard(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint8_t len);
void handle_mouse(uint8_t instance, uint8_t report_id, uint8_t protocol,
                  uint8_t const *report, uint8_t len);
void handle_consumer(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint8_t len);
void handle_uart_generic_msg(uart_packet_t *packet, device_t *state);
void handle_uart_output_select_msg(uart_packet_t *packet, device_t *state);
// keyboard.c
uint8_t get_byte_offset(uint8_t key);
uint8_t get_pos_in_byte(uint8_t key);
bool process_keyboard_report(uint8_t const *report, uint8_t len);
// uart.c
void receive_char(uart_packet_t *packet, device_t *state);
void send_packet(uint8_t instance, uint8_t report_id, const uint8_t *data,
                 enum packet_type_e packet_type, int length);
void send_value(const uint8_t value, enum packet_type_e packet_type);
// usb.c
bool send_tud_report(uint8_t instance, uint8_t report_id, uint8_t const *report,
                     uint8_t len);
bool send_x_report(enum packet_type_e packet_type, uint8_t instance,
                   uint8_t report_id, uint8_t const *report, uint8_t len);
// utils.c
uint8_t calc_checksum(const uint8_t *data, int length);
bool verify_checksum(const uart_packet_t *packet);
// stdio.h
int printf(const char *format, ...);
int puts(const char *s);
// tusb_d.c
enum { ITF_NUM_HID_KB, ITF_NUM_HID_MS, ITF_NUM_HID_CD, ITF_NUM_TOTAL };
/*********  Global variables (don't judge)  **********/
extern device_t global_state;
