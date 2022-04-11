#pragma once

#include "texture.h"

#include <memory>

// Поддерживаемые виды фигур: прямоугольник и эллипс
enum class ShapeType
{
  RECTANGLE, ELLIPSE
};

class Shape
{
  ShapeType shape_type_;
  Point position_;
  Size size_;
  std::shared_ptr<Texture> texture_;
public:
  // Фигура после создания имеет нулевые координаты и размер,
  // а также не имеет текстуры
  explicit Shape(ShapeType type): shape_type_(type)
  {}

  void SetPosition(Point pos)
  {
    position_ = pos;
  }

  void SetSize(Size size)
  {
    size_ = size;
  }

  void SetTexture(std::shared_ptr<Texture> texture)
  {
    texture_ = texture;
  }

  // Рисует фигуру на указанном изображении
  // В зависимости от типа фигуры должен рисоваться либо эллипс, либо прямоугольник
  // Пиксели фигуры, выходящие за пределы текстуры, а также в случае, когда текстура не задана,
  // должны отображаться с помощью символа точка '.'
  // Части фигуры, выходящие за границы объекта image, должны отбрасываться.
  void Draw(Image &image) const
  {
    for (size_t i = 0; i != size_.height; ++i)
    {
      for (size_t j = 0; j != size_.width; ++j)
      {
        if ((position_.y + i < image.size()) && (position_.x + j < image[i].size()))
        {
          image[position_.y + i][position_.x + j] = texture_->GetPixelColor({
                                                  static_cast<int>(j),
                                                  static_cast<int>(i)
                                                });
        }
      }
    }
  }
};
