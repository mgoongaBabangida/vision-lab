"""Generate a target and color, size, shape and hole distractors for the detector."""

from pathlib import Path
import struct


def main():
    width, height = 520, 320
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            disk = (x - 85) ** 2 + (y - 90) ** 2 <= 48 ** 2
            square = 200 <= x <= 280 and 50 <= y <= 130
            ellipse = ((x - 420) / 65) ** 2 + ((y - 90) / 22) ** 2 <= 1
            small = (x - 85) ** 2 + (y - 245) ** 2 <= 7 ** 2
            ring_distance = (x - 240) ** 2 + (y - 245) ** 2
            ring = 24 ** 2 <= ring_distance <= 48 ** 2
            green = (x - 420) ** 2 + (y - 245) ** 2 <= 48 ** 2
            color = (0, 140, 255) if disk or square or ellipse or small or ring else (25, 25, 25)
            if green:
                color = (0, 200, 0)
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'classical-detector-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
