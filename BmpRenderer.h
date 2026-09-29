#pragma once

#include <Arduino.h>
#include <FS.h>
#include <GxEPD2_BW.h>

namespace MonoBmp {

struct Info {
  uint32_t pixelOffset = 0;
  uint32_t rowBytes = 0;
  int width = 0;
  int height = 0;
  bool bottomUp = true;
  bool blackBit = false;
};

inline bool read16(File& file, uint16_t& value) {
  uint8_t bytes[2];
  if (file.read(bytes, sizeof(bytes)) != (int)sizeof(bytes)) return false;
  value = bytes[0] | (bytes[1] << 8);
  return true;
}

inline bool read32(File& file, uint32_t& value) {
  uint8_t bytes[4];
  if (file.read(bytes, sizeof(bytes)) != (int)sizeof(bytes)) return false;
  value = (uint32_t)bytes[0] | ((uint32_t)bytes[1] << 8) |
          ((uint32_t)bytes[2] << 16) | ((uint32_t)bytes[3] << 24);
  return true;
}

inline bool inspect(File& file, Info& info) {
  uint16_t signature, planes, depth;
  uint32_t dibSize, rawWidth, rawHeight, compression;
  if (!read16(file, signature) || signature != 0x4d42 || !file.seek(10) ||
      !read32(file, info.pixelOffset) || !read32(file, dibSize) || dibSize < 40 ||
      !read32(file, rawWidth) || !read32(file, rawHeight) ||
      !read16(file, planes) || !read16(file, depth) || !read32(file, compression)) {
    return false;
  }

  int32_t signedWidth = (int32_t)rawWidth;
  int32_t signedHeight = (int32_t)rawHeight;
  int64_t absoluteHeight = signedHeight < 0 ? -(int64_t)signedHeight : signedHeight;
  if (signedWidth <= 0 || signedWidth > 2000 || absoluteHeight <= 0 ||
      absoluteHeight > 2000 || planes != 1 || depth != 1 || compression != 0) {
    return false;
  }

  info.width = signedWidth;
  info.height = absoluteHeight;
  info.bottomUp = signedHeight > 0;
  info.rowBytes = ((info.width + 31) / 32) * 4;
  const uint64_t fileSize = file.size();
  const uint64_t dibEnd = 14ULL + (uint64_t)dibSize;
  const uint64_t paletteEnd = dibEnd + 8ULL;
  const uint64_t pixelEnd = (uint64_t)info.pixelOffset +
                            (uint64_t)info.rowBytes * (uint64_t)info.height;
  if (info.rowBytes > 260 || dibEnd > fileSize || paletteEnd > fileSize ||
      (uint64_t)info.pixelOffset < paletteEnd || pixelEnd > fileSize) {
    return false;
  }

  uint8_t palette[8];
  if (!file.seek((uint32_t)dibEnd) ||
      file.read(palette, sizeof(palette)) != (int)sizeof(palette)) {
    return false;
  }
  uint16_t luminance0 = palette[2] * 30 + palette[1] * 59 + palette[0] * 11;
  uint16_t luminance1 = palette[6] * 30 + palette[5] * 59 + palette[4] * 11;
  info.blackBit = luminance1 < luminance0;
  return true;
}

inline bool valid(fs::FS& storage, const String& path) {
  File file = storage.open(path, "r");
  if (!file) return false;
  Info info;
  bool ok = inspect(file, info);
  file.close();
  return ok;
}

template <typename Display>
bool draw(fs::FS& storage, Display& display, const String& path,
          int destinationX, int destinationY, int requestedWidth, int requestedHeight) {
  File file = storage.open(path, "r");
  if (!file) return false;
  Info info;
  if (!inspect(file, info)) {
    file.close();
    return false;
  }

  if (requestedWidth <= 0 || requestedHeight <= 0) {
    file.close();
    return false;
  }

  int destinationOffsetX = 0;
  int destinationOffsetY = 0;
  if (destinationX < 0) {
    destinationOffsetX = -destinationX;
    destinationX = 0;
  }
  if (destinationY < 0) {
    destinationOffsetY = -destinationY;
    destinationY = 0;
  }
  int width = requestedWidth - destinationOffsetX;
  int height = requestedHeight - destinationOffsetY;
  width = min(width, 200 - destinationX);
  height = min(height, 200 - destinationY);
  if (width <= 0 || height <= 0) {
    file.close();
    return false;
  }

  uint8_t row[260];
  int cachedImageY = -1;
  for (int y = 0; y < height; y++) {
    int imageY = ((destinationOffsetY + y) * info.height) / requestedHeight;
    imageY = min(imageY, info.height - 1);
    if (imageY != cachedImageY) {
      int storedY = info.bottomUp ? info.height - 1 - imageY : imageY;
      if (!file.seek(info.pixelOffset + (uint32_t)storedY * info.rowBytes) ||
          file.read(row, info.rowBytes) != (int)info.rowBytes) {
        file.close();
        return false;
      }
      cachedImageY = imageY;
    }
    for (int x = 0; x < width; x++) {
      int imageX = ((destinationOffsetX + x) * info.width) / requestedWidth;
      imageX = min(imageX, info.width - 1);
      bool bit = (row[imageX >> 3] & (0x80 >> (imageX & 7))) != 0;
      if (bit == info.blackBit) {
        display.drawPixel(destinationX + x, destinationY + y, GxEPD_BLACK);
      }
    }
  }
  file.close();
  return true;
}

}  // namespace MonoBmp

