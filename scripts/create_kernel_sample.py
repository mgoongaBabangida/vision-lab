"""Create a small binary geometry chart for comparing morphology kernels."""

from pathlib import Path
import struct


def main():
    width, height = 160, 100
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            point = y == 20 and x in (25, 80, 135)
            horizontal = y == 60 and 15 <= x <= 65 and not 38 <= x <= 42
            vertical = x == 115 and 45 <= y <= 90 and not 65 <= y <= 69
            value = 255 if point or horizontal or vertical else 0
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'kernel-shapes-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
