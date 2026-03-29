/*
 * MIT License
 * Copyright (c) 2024 Navaneeth Bhardwaj
 */
#ifndef BLE_UART_H_
#define BLE_UART_H_

#include <stdint.h>
#include <stdbool.h>

/* TLV command types — must match nRF52805 BLE firmware */
#define BLE_CMD_SEND_KEY    0x01  /* value: [keycode, modifier]  */
#define BLE_CMD_SEND_STRING 0x02  /* value: ASCII bytes          */
#define BLE_CMD_CLEAR_BONDS 0x03  /* value: (none)               */
#define BLE_CMD_BLE_CTRL    0x04  /* value: [0x00=off, 0x01=on]  */
#define BLE_CMD_PING        0x05  /* value: (none) — echoes ACK  */
#define BLE_CMD_GET_TIME    0x06  /* value: (none) — reads CTS   */

#define BLE_TLV_MAX_LEN     16

/** Initialise the BLE UART (115200 8N1, A2=TX, A1=RX by default). */
void ble_uart_init(void);

/** Disable the BLE UART to save power before entering standby. */
void ble_uart_deinit(void);

/**
 * Send a TLV frame to the BLE module.
 * @param type  Command type (BLE_CMD_*)
 * @param data  Payload bytes, or NULL if len == 0
 * @param len   Payload length (0–BLE_TLV_MAX_LEN)
 */
void ble_uart_send(uint8_t type, const uint8_t *data, uint8_t len);

/**
 * Non-blocking RX poll — call from main loop or shell command.
 * Returns true and fills *type_out / *data_out / *len_out when a
 * complete frame has been received; returns false otherwise.
 */
bool ble_uart_task(uint8_t *type_out, uint8_t *data_out, uint8_t *len_out);

#endif /* BLE_UART_H_ */
