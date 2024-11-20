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
#include "hardware/watchdog.h"
#include "pico/binary_info.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pio_usb.h"
#include "tusb.h"
#include "tusb_descriptors.h"
#include "user_config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// BOARD CONFIG
#define PICO_A 0
#define PICO_B 1
#define NUM_DEVICES 2          // PICO_A + PICO_B
#define GPIO_LED_PIN 25        // LED is connected to pin 25 on a PICO
#define WATCHDOG_DELAY_MS 500  // milliseconds
#define WATCHDOG_PAUSE_DEBUG 1 // Pause watchdog on debug
#define CORE1_TIMEOUT_US WATCHDOG_DELAY_MS * 1000 // Convert to microseconds

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
  // FIRMWARE_UPGRADE_MSG = 4,
  // MOUSE_ZOOM_MSG = 5,
  // KBD_SET_REPORT_MSG = 6,
  // SWITCH_LOCK_MSG = 7,
  // SYNC_BORDERS_MSG = 8,
  // FLASH_LED_MSG = 9,
  // SCREENSAVER_MSG = 10,
  // WIPE_CONFIG_MSG = 11,
  // SWAP_OUTPUTS_MSG = 12,
  // HEARTBEAT_MSG = 13,
  // OUTPUT_CONFIG_MSG = 14,
  CONSUMER_CONTROL_MSG = 15,
  LOCK_SCREEN_MSG = 16,
  SUSPEND_PC_MSG = 17,
  ENABLE_DEBUG_MSG = 18,
  REQUEST_REBOOT_MSG = 19,
};
typedef enum { IDLE, READING_PACKET, PROCESSING_PACKET } uart_state_t;

enum os_type_e {
  LINUX = 1,
  MACOS,
};

typedef struct {
  uint8_t os;
} device_config_t;

typedef struct {
  uint8_t active_output;         // Currently selected output (0 = A, 1 = B)
  uint64_t core1_last_loop_pass; // when core1 loop went through last
  uint64_t last_activity;        // Timestamp of the last input activity
  bool tud_connected;            // Are we connected to the host
  bool reboot_requested;         // Are we gonna reboot soon
  uart_state_t
      uart_state; // Storing the state for the simple receiver state machine
  device_config_t device_config[NUM_DEVICES];
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
} keyboard_report_t;

typedef struct TU_ATTR_PACKED {
  uint8_t buttons[2]; /**< buttons mask for currently pressed buttons in the
                         mouse. */
  int16_t x;          /**< Current x position of the mouse. */
  int16_t y;          /**< Current y position of the mouse. */
  int8_t wheel;       /**< Current delta wheel movement on the mouse. */
  int8_t pan;         // using AC Pan
} mouse_report_t;

typedef struct TU_ATTR_PACKED {
  uint8_t logitech[4];
  uint8_t apple;
} consumer_report_t;

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
#define INTERFACE_LENGTH 1
#define REPORT_ID_LENGTH 1
#define REPORT_LEN_LENGTH 1
// For simplicity, all packet types are the same length
#define PACKET_DATA_LENGTH 16
#define CHECKSUM_LENGTH 1

#define PACKET_LENGTH                                                          \
  (TYPE_LENGTH + INTERFACE_LENGTH + REPORT_ID_LENGTH + REPORT_LEN_LENGTH +     \
   PACKET_DATA_LENGTH + CHECKSUM_LENGTH)
#define RAW_PACKET_LENGTH (START_LENGTH + PACKET_LENGTH)

/* Data structure defining packets of information transferred */
typedef struct {
  uint8_t type;                     // Enum field describing the type of packet
  uint8_t interface;                // interface
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
void initial_setup(device_t *state);
void setup_uart(void);
void setup_tuh(void);
// actions.c
void _enable_debug(void);
void enable_debug(void);
void lock_screen(void);
void request_reboot(void);
void screensaver_task(device_t *state);
void send_lock_screen_report(uart_packet_t *packet, device_t *state);
void send_suspend_pc_report(uart_packet_t *packet, device_t *state);
void set_onboard_led(device_t *state);
void suspend_pc(void);
void switch_output_a(device_t *state);
void toggle_output(void);
// handlers.c
void handle_keyboard(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint8_t len);
void handle_mouse(uint8_t instance, uint8_t report_id, uint8_t protocol,
                  uint8_t const *report, uint8_t len);
void handle_consumer(uint8_t instance, uint8_t report_id, uint8_t protocol,
                     uint8_t const *report, uint8_t len);
void handle_uart_enable_debug_msg(uart_packet_t *packet, device_t *state);
void handle_uart_generic_msg(uart_packet_t *packet, device_t *state);
void handle_uart_output_select_msg(uart_packet_t *packet, device_t *state);
void handle_uart_request_reboot_msg(uart_packet_t *packet, device_t *state);
// keyboard.c
uint8_t get_byte_offset(uint8_t key);
uint8_t get_pos_in_byte(uint8_t key);
bool process_keyboard_report(uint8_t const *report, uint8_t len);
bool release_all_keys(void);
// uart.c
void uart_receive_char(uart_packet_t *packet, device_t *state);
void uart_send_packet(enum packet_type_e packet_type, uint8_t interface,
                      uint8_t report_id, uint8_t report_len,
                      const uint8_t *data);
void uart_send_value(enum packet_type_e packet_type, const uint8_t value);
// usb.c
bool send_tud_report(uint8_t instance, uint8_t report_id, uint8_t const *report,
                     uint8_t len);
bool send_x_report(enum packet_type_e packet_type, uint8_t interface,
                   uint8_t report_id, uint8_t report_len,
                   uint8_t const *report);
// utils.c
uint8_t calc_checksum(const uint8_t *data, int length);
void kick_watchdog_task(device_t *state);
void set_tud_connected(bool connected);
bool verify_checksum(const uart_packet_t *packet);
// stdio.h
int printf(const char *format, ...);
int puts(const char *s);
// tusb_d.c
enum { ITF_NUM_HID_KB, ITF_NUM_HID_MS, ITF_NUM_HID_CD, ITF_NUM_TOTAL };
/*********  Global variables (don't judge)  **********/
extern device_t global_state;
