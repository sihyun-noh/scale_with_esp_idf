#!/usr/bin/env python3
"""Show only SensorNode FSM heap trace lines from stdin or a serial port."""

import argparse
import re
import sys
from pathlib import Path
from typing import Iterable, Optional, TextIO, Union


HEAP_LOG_MARKERS = ("[FSM][HEAP]", "[DT][HEAP]")
ANSI_ESCAPE_PATTERN = re.compile(r"\x1b\[[0-9;]*m")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Filter SensorNode output to only FSM/data-table heap log lines."
    )
    source = parser.add_mutually_exclusive_group()
    source.add_argument(
        "--port",
        help="Read directly from a serial port instead of stdin.",
    )
    source.add_argument(
        "--list-ports",
        action="store_true",
        help="List available serial ports and exit.",
    )
    parser.add_argument(
        "--baud",
        type=int,
        default=115200,
        help="Serial baud rate (default: 115200).",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Append matching lines to this file while also printing them.",
    )
    return parser.parse_args()


def load_pyserial():
    try:
        import serial
    except ImportError as exc:
        raise SystemExit(
            "pyserial is required for --port/--list-ports. "
            "Run this script in the ESP-IDF Python environment or install pyserial."
        ) from exc
    return serial


def list_serial_ports() -> None:
    load_pyserial()
    from serial.tools import list_ports

    ports = list(list_ports.comports())
    if not ports:
        print("No serial ports found.")
        return

    for port in ports:
        description = port.description or "Unknown device"
        print(f"{port.device}\t{description}")


def serial_lines(port: str, baud: int) -> Iterable[bytes]:
    serial = load_pyserial()
    with serial.Serial(port, baudrate=baud, timeout=1) as connection:
        print(
            f"Filtering {port} at {baud} baud for heap trace logs...",
            file=sys.stderr,
        )
        while True:
            line = connection.readline()
            if line:
                yield line


def filter_heap_logs(
    lines: Iterable[Union[str, bytes]], output_file: Optional[TextIO]
) -> None:
    for raw_line in lines:
        if isinstance(raw_line, bytes):
            line = raw_line.decode("utf-8", errors="replace")
        else:
            line = raw_line

        if not any(marker in line for marker in HEAP_LOG_MARKERS):
            continue

        filtered_line = ANSI_ESCAPE_PATTERN.sub("", line).rstrip("\r\n")
        print(filtered_line, flush=True)
        if output_file is not None:
            output_file.write(filtered_line + "\n")
            output_file.flush()


def main() -> int:
    args = parse_args()

    if args.list_ports:
        list_serial_ports()
        return 0

    output_file = None
    try:
        if args.output is not None:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            output_file = args.output.open("a", encoding="utf-8")

        lines = serial_lines(args.port, args.baud) if args.port else sys.stdin
        filter_heap_logs(lines, output_file)
    except KeyboardInterrupt:
        print("\nStopped.", file=sys.stderr)
    except OSError as exc:
        print(f"I/O error: {exc}", file=sys.stderr)
        return 1
    finally:
        if output_file is not None:
            output_file.close()

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
