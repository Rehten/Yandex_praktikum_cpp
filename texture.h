#pragma once

#include "common.h"

class Texture
{
private:
  static const char default_color_ = '.';
public:
  explicit Texture(Image image)
    : image_(std::move(image))
  {
  }

  Size GetSize() const
  {
    return {image_.size() ? static_cast<int>(image_[0].size()) : 0, static_cast<int>(image_.size())};
  }

  char GetPixelColor(Point p) const
  {
    size_t x = static_cast<size_t>(p.x);
    size_t y = static_cast<size_t>(p.y);

    if (
      image_.empty()
      ||
      (x >= image_[0].size())
      ||
      (y >= image_.size())
      ||
      (p.x < 0)
      ||
      (p.y < 0)
      )
    {
      return default_color_;
    }

    return image_[y][x];
  }

private:
  Image image_;
};
