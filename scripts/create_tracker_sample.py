"""Create an uncompressed AVI using only Python's standard library (about 40 MiB)."""
from pathlib import Path
import math
import struct


def chunk(tag, data):
    return tag + struct.pack('<I', len(data)) + data + (b'\0' if len(data) % 2 else b'')


def listing(tag, data):
    return chunk(b'LIST', tag + data)


def main():
    width, height, fps, count = 320, 240, 30, 180
    frame_size = width * height * 3  # Width is divisible by four: no DIB row padding.
    avih = struct.pack('<14I', 1000000 // fps, frame_size * fps, 0, 16, count, 0, 1, frame_size, width, height, 0, 0, 0, 0)
    strh = struct.pack('<4s4sIHH8I4h', b'vids', b'DIB ', 0, 0, 0, 0, 1, fps, 0, count, frame_size, 0xffffffff, 0, 0, 0, width, height)
    strf = struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, frame_size, 0, 0, 0, 0)
    header = listing(b'hdrl', chunk(b'avih', avih) + listing(b'strl', chunk(b'strh', strh) + chunk(b'strf', strf)))
    frames, index_entries = bytearray(), bytearray()
    for number in range(count):
        pixels = bytearray([25] * frame_size)
        centers = [(int(160 + 110 * math.sin(number * 0.025)), 55),
                   (int(160 - 110 * math.sin(number * 0.025)), 120)]
        if number < 70 or number >= 100:
            centers.append((int(60 + number), 185))
        for cx, cy in centers:
            for y in range(cy - 17, cy + 18):
                for x in range(cx - 17, cx + 18):
                    if (x - cx) ** 2 + (y - cy) ** 2 <= 17 ** 2:
                        offset = ((height - 1 - y) * width + x) * 3
                        pixels[offset:offset + 3] = bytes((0, 140, 255))
        index_entries += struct.pack('<4sIII', b'00db', 16, 4 + len(frames), frame_size)
        frames += chunk(b'00db', pixels)
    body = b'AVI ' + header + listing(b'movi', frames) + chunk(b'idx1', index_entries)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'tracker-sample.avi'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(chunk(b'RIFF', body))
    print(destination)


if __name__ == '__main__':
    main()
