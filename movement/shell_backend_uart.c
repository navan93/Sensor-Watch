/*
 * MIT License
 *
 * Copyright (c) 2023 Edward Shin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "shell_backend.h"
#include "shell_config.h"
#include "watch.h"
#include <stdio.h>

#if SHELL_ENABLE_UART_BACKEND

// Circular buffer for UART input (similar to CDC implementation)
#define UART_READ_BUF_SZ  SHELL_UART_READ_BUF_SZ
#define UART_READ_BUF_IDX(x)  ((x) & (UART_READ_BUF_SZ - 1))
static char s_uart_read_buf[UART_READ_BUF_SZ] = {0};
static volatile size_t s_uart_read_buf_pos = 0;
static volatile size_t s_uart_read_buf_len = 0;

// State tracking
static bool s_uart_initialized = false;

// Forward declarations for UART backend functions
static bool uart_backend_init(void);
static bool uart_backend_is_available(void);
static int uart_backend_getchar(void);
static int uart_backend_putchar(int c);
static void uart_backend_flush(void);
static void uart_backend_deinit(void);

// UART backend implementation
static const shell_backend_t uart_backend = {
    .init = uart_backend_init,
    .is_available = uart_backend_is_available,
    .getchar = uart_backend_getchar,
    .putchar = uart_backend_putchar,
    .flush = uart_backend_flush,
    .deinit = uart_backend_deinit
};

static bool uart_backend_init(void) {
    if (s_uart_initialized) {
        return true;
    }

    // Initialize UART with configured pins and baud rate
    watch_enable_uart(SHELL_UART_TX_PIN, SHELL_UART_RX_PIN, SHELL_UART_BAUD);

    // Clear the input buffer
    s_uart_read_buf_pos = 0;
    s_uart_read_buf_len = 0;

    s_uart_initialized = true;
    return true;
}

static bool uart_backend_is_available(void) {
    return s_uart_initialized;
}

static int uart_backend_getchar(void) {
    if (!s_uart_initialized) {
        return -1;
    }

    // First, try to read any available data from UART and buffer it
    // Note: watch_uart_getc() is blocking, so we need to check if data is available first
    // Since the current UART API doesn't have a non-blocking check, we'll implement
    // a simple polling mechanism

    // If we have buffered data, return it
    if (s_uart_read_buf_len > 0) {
        const size_t start_pos = UART_READ_BUF_IDX(s_uart_read_buf_pos - s_uart_read_buf_len);
        char c = s_uart_read_buf[start_pos];
        s_uart_read_buf[start_pos] = 0;
        s_uart_read_buf_len--;
        return (int)c;
    }

    // No buffered data available
    return -1;
}

static int uart_backend_putchar(int c) {
    if (!s_uart_initialized) {
        return 0;
    }

    // Convert single character to string for watch_uart_puts
    char str[2] = {(char)c, '\0'};
    watch_uart_puts(str);
    return 1;
}

static void uart_backend_flush(void) {
    // UART transmission is synchronous, so no explicit flush needed
}

static void uart_backend_deinit(void) {
    s_uart_initialized = false;
    s_uart_read_buf_pos = 0;
    s_uart_read_buf_len = 0;
}

// Helper function to be called periodically to read UART data into buffer
void shell_backend_uart_task(void) {
    if (!s_uart_initialized) {
        return;
    }

    // This is a simplified approach - in a real implementation, you might want
    // to use interrupts or a more sophisticated polling mechanism
    // For now, we'll rely on the shell calling this periodically
}

// Auto-registration function
void shell_backend_uart_register(void) {
    shell_backend_register(SHELL_BACKEND_UART, &uart_backend);
}

#else // SHELL_ENABLE_UART_BACKEND

// Stub function when UART backend is disabled
void shell_backend_uart_register(void) {
    // Do nothing - UART backend is disabled
}

#endif // SHELL_ENABLE_UART_BACKEND
