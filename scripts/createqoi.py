import struct

# QOI header
magic = b'qoif'
width = 4
height = 4
channels = 4
colorspace = 0

# Pixels:
red = (255, 0, 0, 255)
green = (0, 255, 0, 255)
blue = (0, 0, 255, 255)


qoi_bytes = bytearray()
qoi_bytes += magic
qoi_bytes += struct.pack('>I', width)
qoi_bytes += struct.pack('>I', height)
qoi_bytes += bytes([channels, colorspace])

# Pixel 1: Full RGBA (red)
qoi_bytes.append(0xFF)
qoi_bytes += bytes(red)

# Pixel 2: repeat -> QOI_OP_RUN with run=1
qoi_bytes.append(0xC0)

qoi_bytes.append(0xFF)
qoi_bytes += bytes(blue)

qoi_bytes.append(0xC0)

# Pixel 3: Full RGBA (green)
qoi_bytes.append(0xFF)
qoi_bytes += bytes(green)

# Pixel 4: repeat -> QOI_OP_RUN with run=1
# qoi_bytes.append(0xC0)
qoi_bytes.append(0xC0)
# qoi_bytes.append(0xC3)

qoi_bytes.append(0x32) # QOI_OP_INDEX FOR RED
qoi_bytes.append(0xC2)
# qoi_bytes.append(0xC0)

qoi_bytes.append(0x30) # QOI_OP_INDEX FOR GREEN
qoi_bytes.append(0xC0)
# qoi_bytes.append(0xC0)

qoi_bytes.append(0x2E) # QOI_OP_INDEX FOR BLUE
qoi_bytes.append(0xC2)
# qoi_bytes.append(0xC0)

# End marker
qoi_bytes += bytes([0, 0, 0, 0, 0, 0, 0, 1])

# Save it
with open("dummy.qoi", "wb") as f:
    f.write(qoi_bytes)