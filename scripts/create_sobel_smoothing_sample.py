"""Generate a vertical edge, with controlled alternating-row interference below."""

from pathlib import Path
import struct


def main():
    width, height = 360, 200
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            value = 60 if x < width // 2 else 190
            if y >= height // 2:
                value += 45 * (1 if y % 2 else -1) * (1 if (x // 8) % 2 else -1)
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'sobel-smoothing-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
