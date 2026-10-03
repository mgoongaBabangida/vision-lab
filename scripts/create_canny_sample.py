"""Create connected strong/weak edges and a separate weak object for hysteresis."""

from pathlib import Path
import struct


def main():
    width, height = 400, 240
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            value = 20
            if 60 <= x <= 180 and 30 <= y <= 210:
                value = 220 if y < 90 else max(100, 220 - 3 * (y - 90))
            elif 260 <= x <= 340 and 130 <= y <= 210:
                value = 100
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'canny-hysteresis-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
