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
#include <stddef.h>

// Backend registry
static const shell_backend_t *s_backends[SHELL_BACKEND_COUNT] = {0};
static shell_backend_type_t s_active_backend = SHELL_BACKEND_USB_CDC;
static bool s_initialized = false;

void shell_backend_init(void) {
    if (s_initialized) {
        return;
    }
    
    // Initialize all registered backends
    for (int i = 0; i < SHELL_BACKEND_COUNT; i++) {
        if (s_backends[i] && s_backends[i]->init) {
            s_backends[i]->init();
        }
    }
    
    s_initialized = true;
}

bool shell_backend_set_active(shell_backend_type_t backend_type) {
    if (backend_type >= SHELL_BACKEND_COUNT) {
        return false;
    }
    
    if (!s_backends[backend_type]) {
        return false;
    }
    
    s_active_backend = backend_type;
    return true;
}

shell_backend_type_t shell_backend_get_active(void) {
    return s_active_backend;
}

bool shell_backend_is_available(void) {
    const shell_backend_t *backend = s_backends[s_active_backend];
    if (!backend || !backend->is_available) {
        return false;
    }
    
    return backend->is_available();
}

int shell_backend_getchar(void) {
    const shell_backend_t *backend = s_backends[s_active_backend];
    if (!backend || !backend->getchar) {
        return -1;
    }
    
    return backend->getchar();
}

int shell_backend_putchar(int c) {
    const shell_backend_t *backend = s_backends[s_active_backend];
    if (!backend || !backend->putchar) {
        return 0;
    }
    
    return backend->putchar(c);
}

void shell_backend_flush(void) {
    const shell_backend_t *backend = s_backends[s_active_backend];
    if (backend && backend->flush) {
        backend->flush();
    }
}

void shell_backend_register(shell_backend_type_t backend_type, const shell_backend_t *backend) {
    if (backend_type < SHELL_BACKEND_COUNT) {
        s_backends[backend_type] = backend;
    }
}
