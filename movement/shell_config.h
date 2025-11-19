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

#ifndef SHELL_CONFIG_H_
#define SHELL_CONFIG_H_

#include "watch.h"

/** @brief Enable USB CDC backend for shell (default: enabled) */
#ifndef SHELL_ENABLE_USB_BACKEND
#define SHELL_ENABLE_USB_BACKEND 1
#endif

/** @brief Enable UART backend for shell (default: enabled) */
#ifndef SHELL_ENABLE_UART_BACKEND
#define SHELL_ENABLE_UART_BACKEND 1
#endif

/** @brief Default UART TX pin for shell (default: A2) */
#ifndef SHELL_UART_TX_PIN
#define SHELL_UART_TX_PIN A2
#endif

/** @brief Default UART RX pin for shell (default: A1) */
#ifndef SHELL_UART_RX_PIN
#define SHELL_UART_RX_PIN A1
#endif

/** @brief Default UART baud rate for shell (default: 19200) */
#ifndef SHELL_UART_BAUD
#define SHELL_UART_BAUD 19200
#endif

/** @brief Auto-switch to UART when USB is not available (default: enabled) */
#ifndef SHELL_AUTO_SWITCH_BACKEND
#define SHELL_AUTO_SWITCH_BACKEND 1
#endif

/** @brief Preferred backend when both are available (default: USB CDC) */
#ifndef SHELL_PREFERRED_BACKEND
#define SHELL_PREFERRED_BACKEND SHELL_BACKEND_USB_CDC
#endif

/** @brief Size of UART input buffer (must be power of 2, default: 64) */
#ifndef SHELL_UART_READ_BUF_SZ
#define SHELL_UART_READ_BUF_SZ 64
#endif

#endif
