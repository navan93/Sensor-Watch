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

#if SHELL_ENABLE_USB_BACKEND

// Forward declarations for USB CDC backend functions
static bool usb_backend_init(void);
static bool usb_backend_is_available(void);
static int usb_backend_getchar(void);
static int usb_backend_putchar(int c);
static void usb_backend_flush(void);
static void usb_backend_deinit(void);

// USB CDC backend implementation
static const shell_backend_t usb_backend = {
    .init = usb_backend_init,
    .is_available = usb_backend_is_available,
    .getchar = usb_backend_getchar,
    .putchar = usb_backend_putchar,
    .flush = usb_backend_flush,
    .deinit = usb_backend_deinit
};

static bool usb_backend_init(void) {
    // USB CDC is initialized by the main system, nothing to do here
    return true;
}

static bool usb_backend_is_available(void) {
    // Check if USB is enabled and ready
    return watch_is_usb_enabled();
}

static int usb_backend_getchar(void) {
    // Use the standard getchar() which is implemented by the CDC layer
    return getchar();
}

static int usb_backend_putchar(int c) {
    // Use the standard putchar() which is implemented by the CDC layer
    int result = putchar(c);
    return (result == c) ? 1 : 0;
}

static void usb_backend_flush(void) {
    // Flush is handled automatically by the CDC layer
    // No explicit flush needed
}

static void usb_backend_deinit(void) {
    // USB CDC deinitialization is handled by the main system
    // Nothing to do here
}

// Auto-registration function
void shell_backend_usb_register(void) {
    shell_backend_register(SHELL_BACKEND_USB_CDC, &usb_backend);
}

#else // SHELL_ENABLE_USB_BACKEND

// Stub function when USB backend is disabled
void shell_backend_usb_register(void) {
    // Do nothing - USB backend is disabled
}

#endif // SHELL_ENABLE_USB_BACKEND
