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
private:
  ShapeType shape_type_;
  Point position_;
  Size size_;
  std::shared_ptr<Texture> texture_ptr_;
public:
  // Фигура после создания имеет нулевые координаты и размер,
  // а также не имеет текстуры
  explicit Shape(ShapeType type) : shape_type_(type)
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
    texture_ptr_ = texture;
  }

  // Рисует фигуру на указанном изображении
  // В зависимости от типа фигуры должен рисоваться либо эллипс, либо прямоугольник
  // Пиксели фигуры, выходящие за пределы текстуры, а также в случае, когда текстура не задана,
  // должны отображаться с помощью символа точка '.'
  // Части фигуры, выходящие за границы объекта image, должны отбрасываться.
  void Draw(Image &image) const
  {
    size_t image_height = image.size();
    size_t image_width = image.empty() ? 0 : image[0].size();

    if ((position_.y > static_cast<int>(image_height)) || (position_.x > static_cast<int>(image_width)))
    {
      return;
    }

    size_t start_drawing_y = position_.y > 0 ? static_cast<size_t>(position_.y) : 0;
    size_t start_drawing_x = position_.x > 0 ? static_cast<size_t>(position_.x) : 0;

    auto size = texture_ptr_->GetSize();

    size_t end_drawing_y = start_drawing_y + static_cast<size_t>(size.height) <= image_height
                           ?
                           start_drawing_y + static_cast<size_t>(size.height)
                           : image_height;
    size_t end_drawing_x = start_drawing_x + static_cast<size_t>(size.width) <= image_width
                           ?
                           start_drawing_x + static_cast<size_t>(size.width)
                           : image_width;

    int texture_displacement_y = -1 * position_.y;
    int texture_displacement_x = -1 * position_.x;

    for (size_t i = start_drawing_y; i != end_drawing_y; ++i)
    {
      for (size_t j = start_drawing_x; j != end_drawing_x; ++j)
      {
        Point point{
          static_cast<int>(j) + texture_displacement_x,
          static_cast<int>(i) + texture_displacement_y
        };
        bool is_texture_drawed = shape_type_ == ShapeType::RECTANGLE ? IsPointInRectangle(point, size_) : IsPointInEllipse(point, size_);

        if (is_texture_drawed)
        {
          image[i][j] = texture_ptr_->GetPixelColor(point);
        }
      }
    }
  }
};
