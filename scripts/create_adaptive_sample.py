"""Generate a controlled uneven-illumination sample, without external dependencies."""

from pathlib import Path
import struct


def main():
    width, height = 720, 400
    stride = (width * 3 + 3) & ~3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            background = 60 + round(180 * x / (width - 1))
            local_x, local_y = x % 120, y % 100
            outline = (20 <= local_x <= 100 and (20 <= local_y < 23 or 77 <= local_y < 80))
            outline |= (20 <= local_y < 80 and (20 <= local_x < 23 or 98 <= local_x <= 100))
            stroke = 38 <= local_y < 62 and (40 <= local_x < 43 or 58 <= local_x < 61 or 76 <= local_x < 79)
            value = background - 40 if outline or stroke else background
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'adaptive-lighting-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
