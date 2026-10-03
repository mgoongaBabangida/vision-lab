"""Generate orange shapes, including a ring, for external contour inspection."""

from pathlib import Path
import struct


def main():
    width, height = 440, 240
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            rectangle = 25 <= x <= 110 and 50 <= y <= 190
            radius_squared = (x - 220) ** 2 + (y - 120) ** 2
            ring = 28 ** 2 <= radius_squared <= 65 ** 2
            triangle = 45 <= y <= 195 and abs(x - 360) <= (y - 45) // 2
            color = (0, 140, 255) if rectangle or ring or triangle else (25, 25, 25)
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'contours-shapes-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
