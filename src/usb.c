#include "main.h"

bool send_tud_report(uint8_t instance, uint8_t report_id, uint8_t const *report,
                     uint8_t len) {
  bool success = false;
  if (tud_ready()) {
    success = tud_hid_n_report(instance, report_id, report, len);
    if (!success) {
      printf("x[report] success: %s\r\n", success ? "true" : "false");
    }
  } else {
    printf("x[report] tud not ready\r\n");
  }
  return success;
}

bool send_x_report(enum packet_type_e packet_type, uint8_t instance,
                   uint8_t report_id, uint8_t const *report, uint8_t len) {
  bool success = false;

  if (!len || len > PACKET_DATA_LENGTH) {
    printf("stop flooding on disconnects?\r\n");
    return success;
  }

  if (BOARD_ROLE == global_state.active_output) {
    success = send_tud_report(instance, report_id, report, len);
    if (!success) {
      printf("x[send] instance %d report_id %d type %d size %d\r\n", instance,
             report_id, packet_type, len);
      for (uint8_t i = 0; i < len; i++) {
        printf("%02x ", report[i]);
      }
      printf("\r\n");
    }
  } else {
    send_packet(instance, report_id, (uint8_t *)report, packet_type, len);
  }

  return success;
}
