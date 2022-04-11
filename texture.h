#pragma once

#include "common.h"

class Texture
{
public:
  explicit Texture(Image image)
    : image_(std::move(image))
  {
  }

  Size GetSize() const
  {
    if (image_.empty())
    {
      return {0, 0};
    }

    return {
      static_cast<int>(image_[0].size()),
      static_cast<int>(image_.size())
    };
  }

  char GetPixelColor(Point p) const
  {
    return image_[p.y][p.x];
  }

private:
  Image image_;
};
