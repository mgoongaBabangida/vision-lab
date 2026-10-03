"""Generate a dim, two-population grayscale sample for Otsu thresholding."""

from pathlib import Path
import struct


def main():
    width, height = 720, 400
    stride = (width * 3 + 3) & ~3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            circle = (x - 130) ** 2 + (y - 200) ** 2 <= 75 ** 2
            rectangle = 290 <= x < 430 and 110 <= y < 290
            triangle = 100 <= y <= 290 and abs(x - 585) <= (y - 100) * 0.45
            # Both groups remain below the fixed cutoff of 127.
            texture = ((x * 17 + y * 13) % 7) - 3
            value = (100 if circle or rectangle or triangle else 30) + texture
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes((value, value, value))
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'otsu-dim-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
