"""Create grayscale shapes with horizontal, vertical, diagonal and curved edges."""

from pathlib import Path
import struct


def main():
    width, height = 720, 400
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            value = 20
            if 40 <= x <= 200 and 70 <= y <= 310:
                value = 230
            elif (x - 360) ** 2 + (y - 200) ** 2 <= 110 ** 2:
                value = 150
            elif 80 <= y <= 310 and abs(x - 590) <= (y - 80) // 2:
                value = 65
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'gradient-direction-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
