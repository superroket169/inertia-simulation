#!/usr/bin/env python3
# Sends SMC3 frames like FlyPT Mover does: [A hi lo][B hi lo]

import argparse
import math
import sys
import time


def encode(values, raw_max):
    out = bytearray()
    for name, x in zip(b"AB", values):
        x = max(-1.0, min(1.0, x))
        raw = int((x + 1.0) / 2.0 * raw_max)
        out += bytes([ord("["), name, (raw >> 8) & 0xFF, raw & 0xFF, ord("]")])
    return bytes(out)


def positions(args, t):
    if args.mode == "center":
        return (0.0, 0.0)
    if args.mode == "fixed":
        return (args.a, args.b)
    w = 2.0 * math.pi * args.freq * t
    return (args.amp * math.sin(w), args.amp * math.cos(w))


def main():
    p = argparse.ArgumentParser(description="Send SMC3 frames to the rig firmware.")
    p.add_argument("mode", choices=["center", "fixed", "sine"])
    p.add_argument("--port", default="/dev/ttyUSB0")
    p.add_argument("--baud", type=int, default=250000)
    p.add_argument("--bits", type=int, default=16, help="actuator bit depth, as in Mover")
    p.add_argument("--interval-ms", type=float, default=5.0)
    p.add_argument("--duration", type=float, default=0.0, help="seconds, 0 = until Ctrl-C")
    p.add_argument("-a", type=float, default=0.0, help="fixed mode, axis A in [-1, 1]")
    p.add_argument("-b", type=float, default=0.0, help="fixed mode, axis B in [-1, 1]")
    p.add_argument("--amp", type=float, default=0.3, help="sine mode amplitude in [0, 1]")
    p.add_argument("--freq", type=float, default=0.2, help="sine mode frequency in Hz")
    p.add_argument("--dry-run", type=int, metavar="N", default=0,
                   help="print N frames as hex instead of opening the port")
    args = p.parse_args()

    raw_max = (1 << args.bits) - 1
    dt = args.interval_ms / 1000.0

    if args.dry_run:
        for i in range(args.dry_run):
            frame = encode(positions(args, i * dt), raw_max)
            print(" ".join(f"{c:02X}" for c in frame))
        return

    import serial

    with serial.Serial(args.port, args.baud) as port:
        start = time.monotonic()
        next_send = start
        try:
            while True:
                t = time.monotonic() - start
                if args.duration and t >= args.duration:
                    break
                port.write(encode(positions(args, t), raw_max))
                next_send += dt
                time.sleep(max(0.0, next_send - time.monotonic()))
        except KeyboardInterrupt:
            pass


if __name__ == "__main__":
    sys.exit(main())
