#!/usr/bin/env python3
"""Multi-frame motion detector for the LVGL debug screenshot server.

Takes N screenshots at a given interval and compares consecutive frames
pixel-by-pixel to detect whether the UI is animating / scrolling.

Usage:
    python tools/screenshot/check_motion.py <device-ip>
    python tools/screenshot/check_motion.py <device-ip> --frames 5 --interval 0.3
    python tools/screenshot/check_motion.py <device-ip> --save-frames  # save each frame as PNG

Exit code:
    0  UI is static (all frames identical within threshold)
    1  UI is changing (motion detected)
    2  Error (connection, timeout, …)
"""
import argparse
import os
import socket
import struct
import sys
import time

import numpy as np
from PIL import Image

HEADER_FMT = "<HHHHI"
HEADER_SIZE = struct.calcsize(HEADER_FMT)


def recv_exact(sock: socket.socket, n: int) -> bytes:
    buf = bytearray()
    while len(buf) < n:
        chunk = sock.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("connection closed before receiving all header data")
        buf.extend(chunk)
    return bytes(buf)


def grab_frame(ip: str, port: int, timeout: float) -> np.ndarray:
    """Connect, request one snapshot, return it as an HxWx3 uint8 RGB array."""
    with socket.create_connection((ip, port), timeout=timeout) as sock:
        sock.settimeout(timeout)
        sock.sendall(b"S")
        header = recv_exact(sock, HEADER_SIZE)
        w, h, stride, _, data_len = struct.unpack(HEADER_FMT, header)
        pixel_data = recv_exact(sock, data_len)

    pix = np.frombuffer(pixel_data, dtype="<u2").reshape(h, stride // 2)[:, :w]
    r = ((pix >> 11) & 0x1F).astype(np.uint8)
    g = ((pix >> 5)  & 0x3F).astype(np.uint8)
    b = (pix         & 0x1F).astype(np.uint8)
    r = (r << 3) | (r >> 2)
    g = (g << 2) | (g >> 4)
    b = (b << 3) | (b >> 2)
    return np.dstack((r, g, b))


def diff_stats(a: np.ndarray, b: np.ndarray):
    """Return (changed_pixel_count, changed_pct, mean_delta) between two RGB frames."""
    delta = np.abs(a.astype(np.int16) - b.astype(np.int16)).max(axis=2)  # max channel diff per pixel
    changed = int((delta > 8).sum())   # ignore tiny noise (threshold = 8 out of 255)
    total   = delta.size
    return changed, changed / total * 100.0, float(delta.mean())


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("device_ip")
    parser.add_argument("--port",        type=int,   default=3333)
    parser.add_argument("--frames",      type=int,   default=4,   help="number of frames to capture (default 4)")
    parser.add_argument("--interval",    type=float, default=0.4, help="seconds between frames (default 0.4)")
    parser.add_argument("--threshold",   type=float, default=0.5, help="changed-pixel %% above which motion is declared (default 0.5)")
    parser.add_argument("--timeout",     type=float, default=6.0)
    parser.add_argument("--save-frames", action="store_true",     help="save each frame as PNG alongside this script")
    args = parser.parse_args()

    frames = []
    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "screenshots", "motion")
    if args.save_frames:
        os.makedirs(out_dir, exist_ok=True)

    print(f"Capturing {args.frames} frames every {args.interval}s from {args.device_ip}:{args.port} …")
    for i in range(args.frames):
        try:
            frame = grab_frame(args.device_ip, args.port, args.timeout)
        except Exception as exc:
            print(f"  frame {i+1}: ERROR — {exc}", file=sys.stderr)
            return 2
        frames.append(frame)
        h, w = frame.shape[:2]
        tag = f"frame{i+1:02d}"
        print(f"  {tag}: {w}x{h} captured", end="")
        if args.save_frames:
            path = os.path.join(out_dir, f"{tag}.png")
            Image.fromarray(frame, "RGB").save(path)
            print(f"  →  {path}", end="")
        print()
        if i < args.frames - 1:
            time.sleep(args.interval)

    # Compare consecutive pairs
    print()
    any_motion = False
    for i in range(len(frames) - 1):
        changed_px, changed_pct, mean_delta = diff_stats(frames[i], frames[i + 1])
        moving = changed_pct >= args.threshold
        status = "MOVING ⚠" if moving else "static ✓"
        print(f"  frame{i+1:02d} → frame{i+2:02d}: {changed_px:>7,} px changed "
              f"({changed_pct:5.1f}%)  mean_delta={mean_delta:.1f}  [{status}]")
        if moving:
            any_motion = True

    print()
    if any_motion:
        print("Result: MOTION DETECTED — UI is animating or scrolling.")
        if args.save_frames:
            # Save a side-by-side diff of the pair with the most change
            max_pair = max(range(len(frames)-1),
                           key=lambda j: diff_stats(frames[j], frames[j+1])[0])
            a, b = frames[max_pair], frames[max_pair + 1]
            diff_vis = np.abs(a.astype(np.int16) - b.astype(np.int16)).clip(0, 255).astype(np.uint8)
            # Amplify for visibility
            diff_vis = np.clip(diff_vis * 4, 0, 255).astype(np.uint8)
            path = os.path.join(out_dir, "diff_max.png")
            Image.fromarray(diff_vis, "RGB").save(path)
            print(f"  Amplified diff saved to {path}")
        return 1
    else:
        print("Result: UI is static — no motion detected.")
        return 0


if __name__ == "__main__":
    sys.exit(main())
