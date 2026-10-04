"""Create symmetric and asymmetric shapes for comparing centroid and box center."""

from pathlib import Path
import struct


def main():
    width, height = 520, 240
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            rectangle = 25 <= x <= 135 and 55 <= y <= 185
            triangle = 195 <= x <= 335 and 50 <= y <= 190 and x - 195 + y - 50 <= 140
            l_shape = 385 <= x <= 495 and 50 <= y <= 190 and (x <= 415 or y >= 160)
            color = (0, 140, 255) if rectangle or triangle or l_shape else (25, 25, 25)
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'moments-centroid-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
