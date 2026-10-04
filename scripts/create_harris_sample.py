"""Generate shapes and a checkerboard for local corner detection."""

from pathlib import Path
import struct


def main():
    width, height = 480, 300
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            rectangle = 30 <= x <= 150 and 30 <= y <= 120
            triangle = 175 <= y <= 270 and abs(x - 95) <= (y - 175) * 3 // 4
            circle = (x - 220) ** 2 + (y - 80) ** 2 <= 45 ** 2
            checker = 300 <= x < 450 and 50 <= y < 250 and ((x - 300) // 25 + (y - 50) // 25) % 2 == 0
            value = 230 if rectangle or triangle or circle or checker else 25
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'harris-corners-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
