"""Create colored geometry with small mask defects for Practice 17."""

from pathlib import Path
import struct


def main():
    width, height = 160, 100
    stride = width * 3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            color = (30, 30, 30)
            body = 20 <= x <= 80 and 20 <= y <= 75
            gap = x in (50, 51)
            hole = 34 <= x <= 35 and 40 <= y <= 41
            speck = (x, y) in ((10, 10), (95, 35), (130, 85)) or (100 <= x <= 101 and 12 <= y <= 13)
            larger_patch = 110 <= x <= 114 and 65 <= y <= 69
            if (body and not gap and not hole) or speck or larger_patch:
                color = (0, 140, 255)  # BGR orange, inside the existing HSV range.
            if 105 <= x <= 140 and 25 <= y <= 45:
                color = (255, 80, 0)  # Blue distractor.
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'hsv-cleanup-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
