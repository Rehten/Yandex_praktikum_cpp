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
    if (!texture_) return;

    for (size_t i = 0; i != static_cast<size_t>(size_.height); ++i)
    {
      for (size_t j = 0; j != static_cast<size_t>(size_.width); ++j)
      {
        if ((position_.x + static_cast<int>(j) >= 0) && (position_.y + static_cast<int>(i) >= 0) && (position_.y + i < image.size()) && (position_.x + j < image[i].size()))
        {
          Point point{
            static_cast<int>(j),
            static_cast<int>(i)
          };

          if (shape_type_ == ShapeType::ELLIPSE)
          {
            if (IsPointInEllipse(point, size_))
            {
              image[position_.y + i][position_.x + j] =
                (texture_->GetSize().height >= static_cast<int>(i)) && (texture_->GetSize().width >= static_cast<int>(j)) ? texture_->GetPixelColor(point) : '#';
            }
          }
          else
          {
            image[position_.y + i][position_.x + j] =
              (texture_->GetSize().height >= static_cast<int>(i)) && (texture_->GetSize().width >= static_cast<int>(j)) ? texture_->GetPixelColor(point) : '#';
          }
        }
      }
    }
  }
};
