#!/usr/bin/env python3
"""Read a fixed firmware banner without a pyserial dependency."""

import os
import select
import sys
import termios
import time


def main() -> int:
    if len(sys.argv) != 3:
        print(f"usage: {sys.argv[0]} DEVICE TEXT", file=sys.stderr)
        return 2
    device = sys.argv[1]
    expected = sys.argv[2].encode("ascii")
    descriptor = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
    try:
        settings = termios.tcgetattr(descriptor)
        settings[0] = 0
        settings[1] = 0
        settings[2] = termios.CLOCAL | termios.CREAD | termios.CS8
        settings[3] = 0
        settings[4] = termios.B115200
        settings[5] = termios.B115200
        settings[6][termios.VMIN] = 0
        settings[6][termios.VTIME] = 0
        termios.tcsetattr(descriptor, termios.TCSANOW, settings)
        termios.tcflush(descriptor, termios.TCIFLUSH)
        deadline = time.monotonic() + 6.0
        received = bytearray()
        while time.monotonic() < deadline:
            ready, _, _ = select.select([descriptor], [], [], 0.5)
            if not ready:
                continue
            block = os.read(descriptor, 256)
            received.extend(block)
            if expected in received:
                print(received.decode("ascii", errors="replace"), end="")
                return 0
        print(received.decode("ascii", errors="replace"), end="")
        print(f"missing serial banner {expected!r}", file=sys.stderr)
        return 1
    finally:
        os.close(descriptor)


if __name__ == "__main__":
    raise SystemExit(main())
