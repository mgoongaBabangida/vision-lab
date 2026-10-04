"""Generate nested foreground/background regions and a separate outer object."""

from pathlib import Path
import struct


def main():
    width, height = 520, 360
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            distance = (x - 180) ** 2 + (y - 180) ** 2
            outer_ring = 100 ** 2 <= distance <= 150 ** 2
            island_ring = 15 ** 2 <= distance <= 45 ** 2
            separate = (x - 430) ** 2 + (y - 180) ** 2 <= 55 ** 2
            color = (0, 140, 255) if outer_ring or island_ring or separate else (25, 25, 25)
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'contour-hierarchy-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
