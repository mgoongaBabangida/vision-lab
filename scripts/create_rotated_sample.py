"""Create orange rectangles at different orientations for minimum-area boxes."""

from pathlib import Path
import math
import struct


def main():
    width, height = 440, 280
    stride = width * 3
    rectangles = [(100, 80, 65, 25, 0), (320, 80, 65, 25, 30), (210, 210, 95, 18, -20)]
    transforms = [(cx, cy, hx, hy, math.cos(math.radians(a)), math.sin(math.radians(a)))
                  for cx, cy, hx, hy, a in rectangles]
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            inside = False
            for cx, cy, hx, hy, cosine, sine in transforms:
                dx, dy = x - cx, y - cy
                local_x = cosine * dx + sine * dy
                local_y = -sine * dx + cosine * dy
                inside |= abs(local_x) <= hx and abs(local_y) <= hy
            color = (0, 140, 255) if inside else (25, 25, 25)
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'rotated-rectangles-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
