#!/usr/bin/env python3
"""
list_serial_ports.py - Cross-platform serial port detection.
Usage: python list_serial_ports.py
Output: One port per line, e.g. COM8, /dev/ttyUSB0, /dev/cu.usbserial-110
"""

import sys

try:
    import serial.tools.list_ports
except ImportError:
    print("ERROR: pyserial not installed. Run: pip install pyserial", file=sys.stderr)
    sys.exit(1)


def list_ports():
    ports = serial.tools.list_ports.comports()
    if not ports:
        print("No serial ports found.")
        return

    print(f"Found {len(ports)} serial port(s):")
    print("-" * 50)
    for i, port in enumerate(ports, 1):
        print(f"  {i}. {port.device}")
        print(f"     Description: {port.description}")
        print(f"     Hardware ID: {port.hwid}")
        print()


if __name__ == "__main__":
    list_ports()
