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

#include "shell_cmd_list.h"
#include "shell.h"
#include "shell_backend.h"
#include <stdio.h>
#include <string.h>

static const char* backend_name(shell_backend_type_t backend) {
    switch (backend) {
        case SHELL_BACKEND_USB_CDC:
            return "USB CDC";
        case SHELL_BACKEND_UART:
            return "UART";
        default:
            return "Unknown";
    }
}

int shell_cmd_backend_status(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    shell_backend_type_t current = shell_get_backend();
    printf("Current shell backend: %s", backend_name(current));
    
    // Check availability of each backend
    printf("\nBackend availability:");
    
    // Temporarily switch to check each backend
    shell_backend_type_t original = current;
    
    if (shell_set_backend(SHELL_BACKEND_USB_CDC)) {
        printf("\n  USB CDC: %s", shell_backend_is_available() ? "Available" : "Not available");
        shell_set_backend(original);
    } else {
        printf("\n  USB CDC: Not supported");
    }
    
    if (shell_set_backend(SHELL_BACKEND_UART)) {
        printf("\n  UART: %s", shell_backend_is_available() ? "Available" : "Not available");
        shell_set_backend(original);
    } else {
        printf("\n  UART: Not supported");
    }
    
    return 0;
}

int shell_cmd_backend_switch(int argc, char *argv[]) {
    if (argc != 2) {
        return -2; // Show help
    }
    
    shell_backend_type_t new_backend;
    
    if (strcasecmp(argv[1], "usb") == 0 || strcasecmp(argv[1], "cdc") == 0) {
        new_backend = SHELL_BACKEND_USB_CDC;
    } else if (strcasecmp(argv[1], "uart") == 0) {
        new_backend = SHELL_BACKEND_UART;
    } else {
        printf("Invalid backend: %s", argv[1]);
        return -1;
    }
    
    if (shell_set_backend(new_backend)) {
        printf("Switched to %s backend", backend_name(new_backend));
        if (!shell_backend_is_available()) {
            printf(" (Warning: Backend may not be available)");
        }
    } else {
        printf("Failed to switch to %s backend", backend_name(new_backend));
        return -1;
    }
    
    return 0;
}
