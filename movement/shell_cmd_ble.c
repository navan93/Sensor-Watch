/*
 * MIT License
 * Copyright (c) 2024 Navaneeth Bhardwaj
 */

#include "ble_uart.h"
#include "watch.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Poll for a ping ACK for up to this many milliseconds */
#define PING_TIMEOUT_MS  500

int shell_cmd_ble(int argc, char *argv[]) {
    if (argc < 2) return -2;

    const char *sub = argv[1];

    /* ble ping — send CMD_PING, wait for ACK (05 01 AC) */
    if (strcmp(sub, "ping") == 0) {
        uint8_t type = 0, data[BLE_TLV_MAX_LEN], len = 0;
        ble_uart_send(BLE_CMD_PING, NULL, 0);
        for (int ms = 0; ms < PING_TIMEOUT_MS; ms++) {
            if (ble_uart_task(&type, data, &len)) {
                if (type == BLE_CMD_PING && len == 1 && data[0] == 0xAC) {
                    printf("pong\r\n");
                } else {
                    printf("unexpected: type=0x%02x len=%d\r\n", type, len);
                }
                return 0;
            }
            delay_ms(1);
        }
        printf("timeout\r\n");
        return 0;
    }

    /* ble on — start BLE advertising */
    if (strcmp(sub, "on") == 0) {
        uint8_t v = 0x01;
        ble_uart_send(BLE_CMD_BLE_CTRL, &v, 1);
        printf("BLE on\r\n");
        return 0;
    }

    /* ble off — stop advertising / disconnect */
    if (strcmp(sub, "off") == 0) {
        uint8_t v = 0x00;
        ble_uart_send(BLE_CMD_BLE_CTRL, &v, 1);
        printf("BLE off\r\n");
        return 0;
    }

    /* ble time — request current time via CTS */
    if (strcmp(sub, "time") == 0) {
        ble_uart_send(BLE_CMD_GET_TIME, NULL, 0);
        printf("sent\r\n");
        return 0;
    }

    /* ble bonds — clear all BLE bonds */
    if (strcmp(sub, "bonds") == 0) {
        ble_uart_send(BLE_CMD_CLEAR_BONDS, NULL, 0);
        printf("sent\r\n");
        return 0;
    }

    /* ble str <text> — send ASCII string as HID keypresses */
    if (strcmp(sub, "str") == 0) {
        if (argc < 3) return -2;
        const char *text = argv[2];
        uint8_t len = (uint8_t)strlen(text);
        if (len > BLE_TLV_MAX_LEN) len = BLE_TLV_MAX_LEN;
        ble_uart_send(BLE_CMD_SEND_STRING, (const uint8_t *)text, len);
        printf("sent %d chars\r\n", len);
        return 0;
    }

    /* ble key <keycode> [modifier] — send a single HID key */
    if (strcmp(sub, "key") == 0) {
        if (argc < 3) return -2;
        uint8_t kv[2];
        kv[0] = (uint8_t)strtol(argv[2], NULL, 0);
        kv[1] = (argc >= 4) ? (uint8_t)strtol(argv[3], NULL, 0) : 0;
        ble_uart_send(BLE_CMD_SEND_KEY, kv, (argc >= 4) ? 2 : 1);
        printf("sent key=0x%02x mod=0x%02x\r\n", kv[0], kv[1]);
        return 0;
    }

    return -2;  /* unknown subcommand — shell will print help */
}
