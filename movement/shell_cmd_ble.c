/*
 * MIT License
 * Copyright (c) 2024 Navaneeth Bhardwaj
 */

#include "ble_uart.h"
#include "watch.h"
#include "movement.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Poll for a ping ACK for up to this many milliseconds */
#define PING_TIMEOUT_MS   500
/* CTS round-trip (discovery + read) can take a few seconds */
#define TIME_TIMEOUT_MS  3000
/* Number of timezones in movement_timezone_offsets array */
#define NUM_MOVEMENT_TIMEZONES 41

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

    /* ble time — request current time via CTS, apply to RTC on response */
    if (strcmp(sub, "time") == 0) {
        uint8_t type, data[BLE_TLV_MAX_LEN], len;
        ble_uart_send(BLE_CMD_GET_TIME, NULL, 0);
        printf("Waiting for time response (timeout in 3s)...\r\n");
        for (int ms = 0; ms < TIME_TIMEOUT_MS; ms++) {
            if (ble_uart_task(&type, data, &len)) {
                printf("Received: type=0x%02x len=%d\r\n", type, len);
                if (type == BLE_CMD_GET_TIME && len == 6) {
                    watch_date_time dt;
                    dt.reg = (uint32_t)data[0]
                           | ((uint32_t)data[1] << 8)
                           | ((uint32_t)data[2] << 16)
                           | ((uint32_t)data[3] << 24);
                    watch_rtc_set_date_time(dt);

                    // Parse timezone from bytes 4-5 (int16_t, little-endian)
                    int16_t tz_minutes = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));

                    // Find closest match in movement_timezone_offsets[]
                    uint8_t best_idx = 0;
                    int16_t best_diff = 32767;
                    extern const int16_t movement_timezone_offsets[];
                    extern movement_state_t movement_state;
                    for (uint8_t i = 0; i < NUM_MOVEMENT_TIMEZONES; i++) {
                        int16_t diff = movement_timezone_offsets[i] - tz_minutes;
                        if (diff < 0) diff = -diff;
                        if (diff < best_diff) {
                            best_diff = diff;
                            best_idx = i;
                        }
                    }
                    movement_state.settings.bit.time_zone = best_idx;

                    printf("Rx time bytes: %02X %02X %02X %02X\r\n", data[0], data[1], data[2], data[3]);
                    printf("%04d-%02d-%02d %02d:%02d:%02d\r\n",
                           2020 + dt.unit.year, dt.unit.month, dt.unit.day,
                           dt.unit.hour, dt.unit.minute, dt.unit.second);
                    printf("Timezone: %d min (index %d)\r\n", tz_minutes, best_idx);
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

    /* ble echo — send 4 test bytes, verify the nRF echoes them back */
    if (strcmp(sub, "echo") == 0) {
        static const uint8_t test_bytes[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
        uint8_t type = 0, data[BLE_TLV_MAX_LEN], len = 0;
        ble_uart_send(BLE_CMD_ECHO, test_bytes, 4);
        for (int ms = 0; ms < PING_TIMEOUT_MS; ms++) {
            if (ble_uart_task(&type, data, &len)) {
                if (type == BLE_CMD_ECHO && len == 4) {
                    if (memcmp(data, test_bytes, 4) == 0) {
                        printf("echo OK: %02X %02X %02X %02X\r\n",
                               data[0], data[1], data[2], data[3]);
                    } else {
                        printf("echo MISMATCH: got %02X %02X %02X %02X\r\n",
                               data[0], data[1], data[2], data[3]);
                    }
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
