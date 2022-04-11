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
    return {
      static_cast<int>(image_.size() && image_[0].size()),
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
