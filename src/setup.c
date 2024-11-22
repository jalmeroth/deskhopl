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

void setup_uart(void) {
  // init uart0 for pico-to-pico comms
  gpio_set_function((uint)UART_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function((uint)UART_RX_PIN, GPIO_FUNC_UART);
  uart_init(UART_ZERO, UART_ZERO_BAUD_RATE);
  bi_decl(bi_2pins_with_func(UART_TX_PIN, UART_RX_PIN, GPIO_FUNC_UART));

#ifdef DH_DEBUG
  // init uart1 and configure stdio driver
  stdio_uart_init_full(UART_ONE, UART_ONE_BAUD_RATE, UART_ONE_TX_PIN,
                       UART_ONE_RX_PIN);
  bi_decl(bi_2pins_with_func(UART_ONE_TX_PIN, UART_ONE_RX_PIN, GPIO_FUNC_UART));
#endif
}

void setup_tuh(void) {
  // To run USB SOF interrupt in core1, create alarm pool in core1.
  static pio_usb_configuration_t config = PIO_USB_DEFAULT_CONFIG;
  config.pin_dp = PIO_USB_DP_PIN_DEFAULT;
  config.alarm_pool = (void *)alarm_pool_create(2, 1);

  // We would rather not use the boot protocol
  tuh_hid_set_default_protocol(HID_PROTOCOL_REPORT);

  // Configure TinyUSB to use PIO-USB
  tuh_configure(BOARD_TUH_RHPORT, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &config);

  // Initialize and configure TinyUSB Host
  tuh_init(BOARD_TUH_RHPORT);

  // put pin config into binary_info
  bi_decl(bi_1pin_with_name(PIO_USB_DP_PIN_DEFAULT, "USB DP"));
}

void set_user_config(device_t *state) {
  const char *os_type_str[] = {"undefined", "Linux", "macOS"};

  state->device_config[PICO_A].os = PICO_A_OS;
  state->device_config[PICO_B].os = PICO_B_OS;
  printf("PICO_%s OS: %s\r\n", BOARD_ROLE ? "B" : "A",
         os_type_str[state->device_config[BOARD_ROLE].os]);
}

void initial_setup(device_t *state) {
  // default 125MHz is not appropreate. Sysclock should be multiple of 12MHz.
  set_sys_clock_khz(120000, true);

  // Init and enable the on-board LED GPIO as output
  gpio_init(GPIO_LED_PIN);
  gpio_set_dir(GPIO_LED_PIN, GPIO_OUT);
  bi_decl(bi_1pin_with_name(GPIO_LED_PIN, "LED"));

  setup_uart();

  sleep_ms(10);

  setup_tuh();

  multicore_reset_core1();

  multicore_launch_core1(core1_main);

  set_user_config(state);

  query_active_output(state);
}
