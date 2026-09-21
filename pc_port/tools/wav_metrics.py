#!/usr/bin/env python3
"""Measure peak / RMS / non-zero-frame counts of a 16-bit PCM WAV.

Used by the pe-xa-fidelity evidence run to compare the CD-XA output before and
after the gaussian-interpolation + volume-path change.  No audio golden exists,
so these are signal-level metrics only.

Usage:  python3 pc_port/tools/wav_metrics.py FILE [FILE ...]
"""
import struct
import sys
import wave


def metrics(path):
    with wave.open(path, "rb") as w:
        ch = w.getnchannels()
        sw = w.getsampwidth()
        fr = w.getframerate()
        n = w.getnframes()
        raw = w.readframes(n)
    assert sw == 2, "expected 16-bit PCM, got %d-byte samples" % sw
    samples = struct.unpack("<%dh" % (len(raw) // 2), raw)
    peak = 0
    sumsq = 0
    nonzero = 0
    frames = len(samples) // ch
    for i in range(0, len(samples), ch):
        frame_nonzero = False
        for c in range(ch):
            v = samples[i + c]
            a = v if v >= 0 else -v
            if a > peak:
                peak = a
            sumsq += v * v
            if v != 0:
                frame_nonzero = True
        if frame_nonzero:
            nonzero += 1
    rms = (sumsq / len(samples)) ** 0.5 if samples else 0.0
    return {
        "path": path,
        "channels": ch,
        "rate": fr,
        "frames": frames,
        "peak": peak,
        "rms": rms,
        "nonzero": nonzero,
    }


def main(argv):
    for path in argv[1:]:
        m = metrics(path)
        print("%s: ch=%d rate=%d frames=%d peak=%d rms=%.1f nonzero=%d/%d"
              % (m["path"], m["channels"], m["rate"], m["frames"],
                 m["peak"], m["rms"], m["nonzero"], m["frames"]))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
