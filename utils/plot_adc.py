import serial
import time
import numpy as np
import matplotlib.pyplot as plt

def main():
    # Configure the serial port (adjust settings as needed)
    serial_port = "/dev/ttyACM0"  # Replace with the appropriate port
    baud_rate = 9600  # Match the shell's baud rate
    timeout = 1  # Timeout in seconds for serial reading

    # Initialize serial connection
    with serial.Serial(serial_port, baud_rate, timeout=timeout) as ser:
        try:
            # Clear any initial data in the buffer
            ser.reset_input_buffer()

            # Step 1: Trigger ADC capture
            ser.write(b"adc\n")
            wait_for_response(ser, "Capture ADC samples on A4")

            # Wait for DMA transfer to complete
            wait_for_response(ser, "DMA Transfer completed")

            # Step 2: Fetch the ADC buffer
            ser.write(b"padc\n")
            adc_buffer = wait_for_response(ser, "ADC Buffer:", multiline=True)

            # Parse ADC data from the buffer
            adc_data = parse_adc_data(adc_buffer)
            print(f"Captured {len(adc_data)} samples")

            # Step 3: Perform FFT on the ADC data
            sample_rate = 4_000_000/9  # 1 MHz (adjust based on your ADC configuration)
            freqs, spectrum = perform_fft(adc_data, sample_rate)

            # Step 4: Plot both ADC data and FFT in the same window
            plot_combined_data(adc_data, freqs, spectrum)

        except Exception as e:
            print(f"Error: {e}")

def wait_for_response(ser, expected, multiline=False):
    """
    Waits for an expected response from the serial shell.
    """
    response = []
    while True:
        line = ser.readline().decode('utf-8').strip()
        if line:
            print(f"Received: {line}")
            response.append(line)
            if not multiline and expected in line:
                break
            if multiline and line.startswith(expected):
                response = response[1:]  # Remove prefix
                break
    return response if multiline else response[-1]

def parse_adc_data(buffer_lines):
    """
    Parses the ADC buffer data and converts it to a list of integers.
    """
    adc_values_str = " ".join(buffer_lines).split(":")[1].strip()
    adc_values = [int(val) for val in adc_values_str.split()]
    return adc_values

def perform_fft(adc_data, sample_rate):
    """
    Performs FFT on the ADC data and returns the frequency spectrum.
    """
    # Convert ADC data to a NumPy array
    data = np.array(adc_data, dtype=np.float32)

    # Perform FFT
    fft_result = np.fft.fft(data)
    magnitude = np.abs(fft_result)
    magnitude = magnitude[:len(magnitude) // 2]  # Keep only positive frequencies

    # Generate frequency axis
    freqs = np.fft.fftfreq(len(data), d=1/sample_rate)
    freqs = freqs[:len(magnitude)]  # Positive frequencies only

    # Filter out frequencies below 0.1 MHz
    min_freq = 0.1  # Minimum frequency in MHz
    valid_indices = freqs >= min_freq
    freqs = freqs[valid_indices]
    magnitude = magnitude[valid_indices]

    return freqs, magnitude

def plot_combined_data(adc_data, freqs, spectrum):
    """
    Plots both ADC data and its FFT in the same window.
    """
    fig, axs = plt.subplots(2, 1, figsize=(10, 10))

    # Plot ADC data
    axs[0].plot(adc_data, marker='o', linestyle='-', color='b')
    axs[0].set_title("ADC Data Plot")
    axs[0].set_xlabel("Sample Index")
    axs[0].set_ylabel("ADC Value")
    axs[0].grid(True)

    # Plot FFT data
    axs[1].plot(freqs / 1e6, spectrum, color='r')
    axs[1].set_title("FFT Frequency Spectrum")
    axs[1].set_xlabel("Frequency (MHz)")
    axs[1].set_ylabel("Magnitude")
    axs[1].grid(True)

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()
