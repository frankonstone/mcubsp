#!/usr/bin/env python3
"""Read a named banner from an explicitly selected serial port."""
import argparse
import math
import os
import select
import sys
import time


def scan(reader, banner, timeout, clock=time.monotonic):
    if not banner or len(banner) > 4096 or not math.isfinite(timeout) or timeout <= 0:
        raise ValueError('banner must be 1..4096 bytes and timeout must be positive and finite')
    deadline = clock() + timeout
    window = b''
    while clock() < deadline:
        chunk = reader(min(0.1, max(0, deadline - clock())))
        window = (window + chunk)[-max(8192, 2 * len(banner)):]
        if banner in window:
            return
    raise TimeoutError(f'serial banner was not observed within {timeout:g} seconds')


def observe(port, banner, timeout, baud):
    if os.name == 'posix':
        import termios
        import tty
        speed = getattr(termios, f'B{baud}', None)
        if speed is None:
            raise ValueError(f'unsupported serial rate: {baud}')
        fd = os.open(port, os.O_RDONLY | os.O_NOCTTY | os.O_NONBLOCK)
        original = None
        try:
            original = termios.tcgetattr(fd)
            tty.setraw(fd)
            settings = termios.tcgetattr(fd)
            settings[4] = settings[5] = speed
            termios.tcsetattr(fd, termios.TCSANOW, settings)
            def read(wait):
                ready, _, _ = select.select([fd], [], [], wait)
                return os.read(fd, 4096) if ready else b''
            scan(read, banner, timeout)
        finally:
            if original is not None:
                termios.tcsetattr(fd, termios.TCSANOW, original)
            os.close(fd)
    else:
        try:
            import serial
        except ImportError as error:
            raise ValueError('Windows serial capture requires pyserial in the selected Python environment') from error
        with serial.Serial(port, baudrate=baud, timeout=0.1) as stream:
            scan(lambda wait: stream.read(4096), banner, timeout)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('port')
    parser.add_argument('banner')
    parser.add_argument('--timeout', type=float, default=5.0)
    parser.add_argument('--baud', type=int, default=115200)
    args = parser.parse_args()
    try:
        observe(args.port, args.banner.encode(), args.timeout, args.baud)
    except (ValueError, OSError, TimeoutError) as error:
        parser.exit(2, f'mcubsp serial: {error}\n')
    print(args.banner)


if __name__ == '__main__':
    main()
