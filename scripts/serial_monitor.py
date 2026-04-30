#!/usr/bin/env python3
"""
serial_monitor.py - Cross-platform serial port monitor with log filtering and auto-reconnect.
Usage:
    python serial_monitor.py                          # Auto-detect port, default baud
    python serial_monitor.py -p COM8                  # Specific port
    python serial_monitor.py -p COM8 -b 115200        # Specific port + baud
    python serial_monitor.py --filter "ERROR|WARN"    # Filter lines
    python serial_monitor.py --log monitor.log        # Save to file

Windows notes:
- Uses COM<n> format (e.g., COM8), NOT /dev/ttyUSB0
- Auto-reconnect handles USB disconnect/reconnect on Windows
- Port locking: close other terminals using the port first
"""

import argparse
import sys
import time
import re

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("ERROR: pyserial not installed. Run: pip install pyserial", file=sys.stderr)
    sys.exit(1)


def find_esp_port():
    """Auto-detect ESP32 serial port."""
    ports = serial.tools.list_ports.comports()
    esp_keywords = ["usb serial", "usb-jtag", "cp210", "ch340", "silicon", "ftdi", "usb-to-uart"]

    for port in ports:
        desc_lower = port.description.lower()
        hwid_lower = port.hwid.lower()
        for kw in esp_keywords:
            if kw in desc_lower or kw in hwid_lower:
                return port.device

    # Fallback: return first available port
    if ports:
        return ports[0].device
    return None


def main():
    parser = argparse.ArgumentParser(description="ESP-IDF Serial Monitor with auto-reconnect")
    parser.add_argument("-p", "--port", help="Serial port (e.g., COM8, /dev/ttyUSB0)")
    parser.add_argument("-b", "--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--filter", help="Regex filter for output lines")
    parser.add_argument("--log", help="Log file path")
    parser.add_argument("--reconnect-delay", type=int, default=3, help="Seconds between reconnect attempts")
    args = parser.parse_args()

    # Determine port
    port = args.port
    if not port:
        port = find_esp_port()
        if not port:
            print("ERROR: No serial port found. Specify with -p COM<n>.", file=sys.stderr)
            sys.exit(1)
        print(f"Auto-detected port: {port}")

    # Compile filter
    line_filter = None
    if args.filter:
        try:
            line_filter = re.compile(args.filter, re.IGNORECASE)
        except re.error as e:
            print(f"ERROR: Invalid regex filter: {e}", file=sys.stderr)
            sys.exit(1)

    print(f"Connecting to {port} at {args.baud} baud...")
    print("Press Ctrl+C to exit.\n")

    log_file = open(args.log, "a", encoding="utf-8") if args.log else None

    try:
        while True:
            try:
                ser = serial.Serial(port, args.baud, timeout=1)
                print(f"Connected to {port}.\n{'='*40}")

                while True:
                    try:
                        line = ser.readline().decode("utf-8", errors="replace")
                        if not line:
                            # Check if port still open
                            if not ser.is_open:
                                print("\nPort closed. Reconnecting...")
                                break
                            continue

                        # Apply filter
                        if line_filter and not line_filter.search(line):
                            continue

                        sys.stdout.write(line)
                        sys.stdout.flush()

                        if log_file:
                            log_file.write(line)
                            log_file.flush()

                    except serial.SerialException as e:
                        print(f"\nSerial error: {e}")
                        break

                ser.close()

            except serial.SerialException as e:
                print(f"Cannot open {port}: {e}")

            print(f"Reconnecting in {args.reconnect_delay}s...")
            time.sleep(args.reconnect_delay)

    except KeyboardInterrupt:
        print("\nMonitor stopped.")
    finally:
        if log_file:
            log_file.close()


if __name__ == "__main__":
    main()
