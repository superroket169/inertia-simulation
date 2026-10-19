#!/usr/bin/env python3
# Builds smc3.c for the host and feeds it bytes through ctypes.

import ctypes
import math
import pathlib
import subprocess
import tempfile
import unittest

from smc3_send import encode

FIRMWARE_DIR = pathlib.Path(__file__).resolve().parent.parent


class Smc3Parser(ctypes.Structure):
    _fields_ = [
        ("state", ctypes.c_uint8),
        ("axis", ctypes.c_uint8),
        ("value", ctypes.c_uint16),
    ]


def build_lib(out_dir):
    lib_path = pathlib.Path(out_dir) / "libsmc3.so"
    subprocess.run(
        ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-shared", "-fPIC",
         str(FIRMWARE_DIR / "smc3.c"), "-o", str(lib_path)],
        check=True,
    )
    lib = ctypes.CDLL(str(lib_path))
    lib.smc3_feed.argtypes = [ctypes.POINTER(Smc3Parser), ctypes.c_uint8]
    lib.smc3_feed.restype = ctypes.c_bool
    return lib


class TestSmc3(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.tmp = tempfile.TemporaryDirectory()
        cls.lib = build_lib(cls.tmp.name)

    @classmethod
    def tearDownClass(cls):
        cls.tmp.cleanup()

    def setUp(self):
        self.parser = Smc3Parser()

    def feed(self, data):
        frames = []
        for c in data:
            if self.lib.smc3_feed(ctypes.byref(self.parser), c):
                frames.append((self.parser.axis, self.parser.value))
        return frames

    def test_single_frame(self):
        self.assertEqual(self.feed(b"[A\x12\x34]"), [(0, 0x1234)])

    def test_completes_only_on_closing_bracket(self):
        self.assertEqual(self.feed(b"[B\x00\x01"), [])
        self.assertEqual(self.feed(b"]"), [(1, 0x0001)])

    def test_bracket_bytes_inside_value(self):
        self.assertEqual(self.feed(b"[A[]]"), [(0, 0x5B5D)])

    def test_garbage_before_frame(self):
        self.assertEqual(self.feed(b"\x00\xffxyz][A\x00\x10]"), [(0, 0x0010)])

    def test_unknown_axis_rejected(self):
        self.assertEqual(self.feed(b"[C\x12\x34][A\x00\x02]"), [(0, 0x0002)])

    def test_missing_closing_bracket_drops_frame(self):
        self.assertEqual(self.feed(b"[A\x12\x34X[B\x00\x07]"), [(1, 0x0007)])

    def test_truncated_frame_loses_at_most_next_frame(self):
        data = b"[A\x01" + b"[B\x00\x10]" + b"[A\x00\x20]"
        self.assertEqual(self.feed(data), [(0, 0x0020)])

    def test_sender_stream_roundtrip(self):
        raw_max = 0xFFFF
        expected = []
        stream = b""
        for i in range(200):
            w = 2.0 * math.pi * i / 50.0
            values = (math.sin(w), math.cos(w))
            stream += encode(values, raw_max)
            expected += [(axis, int((x + 1.0) / 2.0 * raw_max)) for axis, x in enumerate(values)]
        self.assertEqual(self.feed(stream), expected)


if __name__ == "__main__":
    unittest.main()
