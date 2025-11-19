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

#ifndef SHELL_BACKEND_H_
#define SHELL_BACKEND_H_

#include <stdint.h>
#include <stdbool.h>

/** @brief Shell backend types */
typedef enum {
    SHELL_BACKEND_USB_CDC = 0,
    SHELL_BACKEND_UART = 1,
    SHELL_BACKEND_COUNT
} shell_backend_type_t;

/** @brief Shell backend interface structure */
typedef struct {
    /** @brief Initialize the backend */
    bool (*init)(void);
    
    /** @brief Check if backend is available/ready */
    bool (*is_available)(void);
    
    /** @brief Read a character from the backend (non-blocking)
     *  @return character read, or -1 if no data available */
    int (*getchar)(void);
    
    /** @brief Write a character to the backend
     *  @param c character to write
     *  @return number of characters written (0 or 1) */
    int (*putchar)(int c);
    
    /** @brief Flush any pending output */
    void (*flush)(void);
    
    /** @brief Deinitialize the backend */
    void (*deinit)(void);
} shell_backend_t;

/** @brief Initialize shell backend system */
void shell_backend_init(void);

/** @brief Set the active shell backend
 *  @param backend_type The backend type to activate
 *  @return true if backend was successfully activated */
bool shell_backend_set_active(shell_backend_type_t backend_type);

/** @brief Get the currently active backend type
 *  @return The active backend type */
shell_backend_type_t shell_backend_get_active(void);

/** @brief Check if any backend is available
 *  @return true if at least one backend is available */
bool shell_backend_is_available(void);

/** @brief Read a character from the active backend
 *  @return character read, or -1 if no data available */
int shell_backend_getchar(void);

/** @brief Write a character to the active backend
 *  @param c character to write
 *  @return number of characters written */
int shell_backend_putchar(int c);

/** @brief Flush output on the active backend */
void shell_backend_flush(void);

/** @brief Register a backend implementation
 *  @param backend_type The backend type
 *  @param backend Pointer to backend implementation */
void shell_backend_register(shell_backend_type_t backend_type, const shell_backend_t *backend);

#endif
