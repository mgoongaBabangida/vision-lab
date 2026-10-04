"""Generate reproducible, varied texture for the SIFT/ORB matching experiment."""
from pathlib import Path
import random
import struct


def main():
    width, height = 480, 320
    pixels = bytearray([25] * (width * height * 3))
    rng = random.Random(27)
    for index in range(220):
        cx, cy = rng.randrange(24, width - 24), rng.randrange(24, height - 24)
        radius = rng.randrange(3, 15)
        color = bytes(rng.randrange(70, 256) for _ in range(3))
        for y in range(cy - radius, cy + radius + 1):
            for x in range(cx - radius, cx + radius + 1):
                distance = (x - cx) ** 2 + (y - cy) ** 2
                if index % 3 == 0 or distance <= radius ** 2:
                    if index % 3 != 2 or distance >= (radius - 2) ** 2:
                        offset = ((height - 1 - y) * width + x) * 3
                        pixels[offset:offset + 3] = color
    header = struct.pack('<2sIHHI', b'BM', 54 + len(pixels), 0, 0, 54)
    header += struct.pack('<IiiHHIIiiII', 40, width, height, 1, 24, 0, len(pixels), 2835, 2835, 0, 0)
    destination = Path(__file__).resolve().parents[1] / 'data' / 'feature-matching-sample.bmp'
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as output:
        output.write(header)
        output.write(pixels)
    print(destination)


if __name__ == '__main__':
    main()
