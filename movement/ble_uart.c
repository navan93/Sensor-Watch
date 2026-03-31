/*
 * MIT License
 * Copyright (c) 2024 Navaneeth Bhardwaj
 */

#include "ble_uart.h"
#include "watch_uart.h"   /* watch_enable_uart(); pulls in watch.h → driver_init.h → SERCOM defs */
#include "hal_io.h"       /* io_write() / io_descriptor */

/* ---- Pin / baud configuration ---- */
#ifndef BLE_UART_TX_PIN
#define BLE_UART_TX_PIN  A2   /* watch→BLE: PB02 ↔ nRF P0.16 (nRF RX) */
#endif
#ifndef BLE_UART_RX_PIN
#define BLE_UART_RX_PIN  A1   /* BLE→watch: PB01 ↔ nRF P0.18 (nRF TX) */
#endif
#define BLE_UART_BAUD    9600

/* TX uses the same ASF4 io path as watch_uart_puts(), but binary-safe (no strlen). */
extern struct io_descriptor *uart_io;

/* Non-blocking: true when a received byte is waiting in the shift register. */
static bool rx_ready(void) {
    return !!(SERCOM3->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC);
}

/* Read one byte — only call when rx_ready() is true. */
static uint8_t rx_byte(void) {
    return (uint8_t)SERCOM3->USART.DATA.reg;
}

/* ---- TLV receive state machine ---- */

typedef enum { TLV_TYPE, TLV_LENGTH, TLV_VALUE } tlv_state_t;

static tlv_state_t  s_state;
static uint8_t      s_type;
static volatile uint8_t      s_length;
static volatile uint8_t      s_value[BLE_TLV_MAX_LEN];
static volatile uint8_t      s_idx;

/* Double-buffer: ISR fills s_*, task reads s_pending_* */
static bool         s_frame_ready;
static uint8_t      s_pending_type;
static volatile uint8_t      s_pending_data[BLE_TLV_MAX_LEN];
static volatile uint8_t      s_pending_len;

static bool         s_uart_enabled;

/* ---- Public API ---- */

void ble_uart_init(void) {
    if (s_uart_enabled) return;
    watch_enable_uart(BLE_UART_TX_PIN, BLE_UART_RX_PIN, BLE_UART_BAUD);
    s_state        = TLV_TYPE;
    s_frame_ready  = false;
    s_uart_enabled = true;
}

void ble_uart_deinit(void) {
    if (!s_uart_enabled) return;

    /* Wait for any in-progress transmission to complete */
    while (!(SERCOM3->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_TXC));

    /* Disable the SERCOM3 peripheral and wait for sync */
    SERCOM3->USART.CTRLA.reg &= ~SERCOM_USART_CTRLA_ENABLE;
    while (SERCOM3->USART.SYNCBUSY.reg & SERCOM_USART_SYNCBUSY_ENABLE);

    /* Gate off the SERCOM3 bus clock */
    MCLK->APBCMASK.reg &= ~MCLK_APBCMASK_SERCOM3;

    /* Return pins to high-impedance inputs to prevent current leakage */
    gpio_set_pin_function(BLE_UART_TX_PIN, GPIO_PIN_FUNCTION_OFF);
    gpio_set_pin_function(BLE_UART_RX_PIN, GPIO_PIN_FUNCTION_OFF);

    s_uart_enabled = false;
    s_state        = TLV_TYPE;
    s_frame_ready  = false;
}

void ble_uart_send(uint8_t type, const uint8_t *data, uint8_t len) {
    if (!s_uart_enabled) return;
    uint8_t frame[2 + BLE_TLV_MAX_LEN];
    frame[0] = type;
    frame[1] = len;
    for (uint8_t i = 0; i < len; i++) frame[2 + i] = data[i];
    io_write(uart_io, frame, 2 + len);
}

bool ble_uart_task(uint8_t *type_out, uint8_t *data_out, uint8_t *len_out) {
    if (!s_uart_enabled) return false;
    /* Drain all available bytes through the TLV state machine */
    while (rx_ready()) {
        uint8_t byte = rx_byte();

        switch (s_state) {
        case TLV_TYPE:
            s_type  = byte;
            s_state = TLV_LENGTH;
            continue;

        case TLV_LENGTH:
            if (byte > BLE_TLV_MAX_LEN) {
                s_state = TLV_TYPE;  /* invalid length — reset */
                continue;
            }
            s_length = byte;
            s_idx    = 0;
            if (byte > 0) {
                s_state = TLV_VALUE;
                continue;
            }
            /* zero-length frame — fall through to dispatch */
            break;

        case TLV_VALUE:
            s_value[s_idx++] = byte;
            if (s_idx < s_length) continue;
            /* all value bytes received — fall through to dispatch */
            break;
        }

        /* Frame complete: copy to pending slot */
        s_pending_type = s_type;
        s_pending_len  = s_length;
        for (uint8_t i = 0; i < s_length; i++) s_pending_data[i] = s_value[i];
        s_frame_ready  = true;
        s_state        = TLV_TYPE;
    }

    if (!s_frame_ready) return false;

    s_frame_ready = false;
    if (type_out) *type_out = s_pending_type;
    if (len_out)  *len_out  = s_pending_len;
    if (data_out) {
        for (uint8_t i = 0; i < s_pending_len; i++) data_out[i] = s_pending_data[i];
    }
    return true;
}
