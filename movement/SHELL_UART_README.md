# UART Shell Backend for Sensor Watch

This implementation adds UART as a backend for the shell system, allowing the shell to work over both USB CDC and UART interfaces.

## Features

- **Dual Backend Support**: Shell can work over USB CDC (when connected to USB) or UART (when using external serial connection)
- **Automatic Backend Selection**: Intelligently switches between backends based on availability
- **Runtime Backend Switching**: Commands to check status and switch backends during runtime
- **Configurable UART Parameters**: Customizable TX/RX pins and baud rate
- **Backward Compatibility**: Existing shell commands work unchanged

## Configuration

The UART shell backend can be configured through compile-time definitions in `shell_config.h`:

### Basic Configuration
```c
#define SHELL_ENABLE_USB_BACKEND 1      // Enable USB CDC backend (default: 1)
#define SHELL_ENABLE_UART_BACKEND 1     // Enable UART backend (default: 1)
#define SHELL_UART_TX_PIN A2            // UART TX pin (default: A2)
#define SHELL_UART_RX_PIN A1            // UART RX pin (default: A1)
#define SHELL_UART_BAUD 19200           // UART baud rate (default: 19200)
```

### Advanced Configuration
```c
#define SHELL_AUTO_SWITCH_BACKEND 1                    // Auto-switch based on availability
#define SHELL_PREFERRED_BACKEND SHELL_BACKEND_USB_CDC  // Preferred backend when both available
#define SHELL_UART_READ_BUF_SZ 64                      // UART input buffer size (power of 2)
```

## Hardware Setup

### UART Connection
Connect your serial adapter to the configured pins:
- **TX Pin** (default A2): Connect to RX of your serial adapter
- **RX Pin** (default A1): Connect to TX of your serial adapter
- **Ground**: Connect to ground of your serial adapter

### Supported Pins
- **TX Pins**: A2, A4
- **RX Pins**: A1, A2, A3, A4 (A0 cannot receive UART data)

## Usage

### Shell Commands

#### Check Backend Status
```
swsh> backend
Current shell backend: USB CDC
Backend availability:
  USB CDC: Available
  UART: Available
```

#### Switch Backend
```
swsh> switch uart
Switched to UART backend

swsh> switch usb
Switched to USB CDC backend
```

### Backend Selection Logic

1. **Auto-Switch Mode** (default): 
   - Uses preferred backend if available
   - Falls back to alternative if preferred is unavailable
   - Default preference: USB CDC > UART

2. **Manual Mode**: 
   - Uses specified backend regardless of availability
   - Useful for testing or specific deployment scenarios

## Integration

### In Your Application

The shell system initializes automatically when `shell_init()` is called (done automatically in `app_setup()`).

### Custom Backend Selection

```c
#include "shell.h"

// Force UART backend
shell_set_backend(SHELL_BACKEND_UART);

// Check current backend
shell_backend_type_t current = shell_get_backend();
```

## Implementation Details

### Architecture
- **Backend Abstraction**: Clean interface allowing multiple communication backends
- **Non-blocking I/O**: Shell polling doesn't block the main application loop
- **Circular Buffering**: Efficient handling of UART input data
- **Conditional Compilation**: Backends can be disabled to save memory

### Files Added
- `shell_backend.h/c`: Core backend abstraction
- `shell_backend_usb.c`: USB CDC backend implementation  
- `shell_backend_uart.c`: UART backend implementation
- `shell_cmd_backend.c`: Backend management commands
- `shell_config.h`: Configuration options

### Files Modified
- `shell.h/c`: Updated to use backend abstraction
- `shell_cmd_list.c`: Added backend management commands
- `movement.c`: Initialize shell system and handle both backends

## Testing

1. **USB CDC Test**: Connect via USB and verify shell works as before
2. **UART Test**: Connect serial adapter and verify shell works over UART
3. **Backend Switching**: Test runtime switching between backends
4. **Configuration Test**: Verify different pin and baud rate configurations

## Troubleshooting

### UART Not Working
- Check pin configuration matches your hardware setup
- Verify baud rate matches your serial terminal
- Ensure proper ground connection
- Check that UART backend is enabled in configuration

### Backend Switching Issues
- Verify both backends are enabled and available
- Check that the target backend hardware is properly connected
- Use `backend` command to check availability status

## Future Enhancements

- **Interrupt-driven UART**: More efficient UART handling with interrupts
- **Multiple UART Instances**: Support for multiple UART backends
- **Bluetooth Backend**: Wireless shell access
- **Network Backend**: TCP/IP shell access for WiFi-enabled variants
