"""Generate a local HSV exercise image using only the Python standard library."""

from pathlib import Path
import struct


def main():
    width, height = 720, 400
    # BGR colors: bright orange, dim orange, pale orange, white, green, blue.
    colors = [(0, 128, 255), (0, 64, 128), (205, 230, 255), (255, 255, 255), (0, 255, 0), (255, 0, 0)]
    centers = [(120, 100), (360, 100), (600, 100), (120, 300), (360, 300), (600, 300)]
    stride = (width * 3 + 3) & ~3
    pixels = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            color = (32, 32, 32)
            for (cx, cy), candidate in zip(centers, colors):
                if (x - cx) ** 2 + (y - cy) ** 2 <= 70 ** 2:
                    color = candidate
                    break
            offset = (height - 1 - y) * stride + x * 3
            pixels[offset:offset + 3] = bytes(color)
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'hsv-color-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
