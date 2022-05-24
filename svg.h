#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <unordered_map>
#include <exception>
#include <variant>

namespace svg {

struct Rgb;
struct Rgba;

using Color = std::variant<std::monostate, Rgb, Rgba, std::string>;

bool operator != (Color, std::string &);
bool operator != (Color, const std::string &);

struct Rgb {
  uint8_t red;
  uint8_t green;
  uint8_t blue;

  Rgb() : Rgb(0, 0, 0) {}
  Rgb(uint8_t red, uint8_t green, uint8_t blue) : red(red), green(green), blue(blue) {}
};

struct Rgba : Rgb {
  double opacity;

  Rgba() : Rgba(0, 0, 0, 1.0) {}
  Rgba(uint8_t red, uint8_t green, uint8_t blue, double alpha) : Rgb(red, green, blue), opacity(alpha) {}
};

std::ostream &operator <<(std::ostream &os, Color color);

class ColorStringifier {
 public:
  std::string operator()(std::monostate);
  std::string operator()(Rgb &);
  std::string operator()(Rgba &);
  std::string operator()(std::string &);
};

const std::string NoneColor = "none";

enum class StrokeLineCap {
  BUTT,
  ROUND,
  SQUARE,
};

enum class StrokeLineJoin {
  ARCS,
  BEVEL,
  MITER,
  MITER_CLIP,
  ROUND,
};

std::ostream &operator <<(std::ostream &os, StrokeLineCap cap);
std::ostream &operator <<(std::ostream &os, StrokeLineJoin join);

struct Point {
  Point() = default;
  Point(double x, double y)
	  : x(x), y(y) {
  }
  double x = 0;
  double y = 0;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
  RenderContext(std::ostream &out = std::cout) : out(out) {
  }

  RenderContext(std::ostream &out, int indent_step, int indent = 0)
	  : out(out), indent_step(indent_step), indent(indent) {
  }

  RenderContext Indented() const {
	return {out, indent_step, indent + indent_step};
  }

  void RenderIndent() const {
	for (int i = 0; i < indent; ++i) {
	  out.put(' ');
	}
  }

  std::ostream &out;
  int indent_step = 0;
  int indent = 0;
};

template<typename Owner>
class PathProps {
  std::optional<Color> fill_color_ = std::nullopt;
  std::optional<Color> stroke_color_ = std::nullopt;
  std::optional<std::string> stroke_width_ = std::nullopt;
  std::optional<StrokeLineCap> stroke_line_cap_ = std::nullopt;
  std::optional<StrokeLineJoin> stroke_line_join_ = std::nullopt;
 public:
  Owner &SetFillColor(const Color &fill_color) {
	fill_color_ = fill_color;

	return AsOwner();
  }
  Owner &SetStrokeColor(const Color &stroke_color) {
	stroke_color_ = stroke_color;

	return AsOwner();
  }
  Owner &SetStrokeWidth(const std::string &stroke_width) {
	stroke_width_ = stroke_width;

	return AsOwner();
  }
  Owner &SetStrokeWidth(int stroke_width) {
	stroke_width_ = std::to_string(stroke_width);

	return AsOwner();
  }
  Owner &SetStrokeLineCap(StrokeLineCap stroke_line_cap) {
	stroke_line_cap_ = stroke_line_cap;

	return AsOwner();
  }
  Owner &SetStrokeLineJoin(StrokeLineJoin stroke_line_join) {
	stroke_line_join_ = stroke_line_join;

	return AsOwner();
  }

  void RenderAttrs(std::ostream &os) const {
	if (fill_color_ && (fill_color_.value() != NoneColor)) os << " fill=\"" << fill_color_.value() << "\"";
	if (stroke_color_ && (stroke_color_.value() != NoneColor)) os << " stroke=\"" << stroke_color_.value() << "\"";
	if (stroke_width_) os << " stroke-width=\"" << stroke_width_.value() << "\"";
	if (stroke_line_cap_) os << " stroke-linecap=\"" << stroke_line_cap_.value() << "\"";
	if (stroke_line_join_) os << " stroke-linejoin=\"" << stroke_line_join_.value() << "\"";
  }
  virtual ~PathProps() = default;
 private:
  Owner &AsOwner() {
	return static_cast<Owner &>(*this);
  }
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
 public:
  void Render(const RenderContext &context) const;

  virtual ~Object() = default;

 private:
  virtual void RenderObject(const RenderContext &context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
 public:
  Circle &SetCenter(Point center);
  Circle &SetRadius(double radius);

 private:
  void RenderObject(const RenderContext &context) const override;

  Point center_;
  double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline : public Object, public PathProps<Polyline> {
 public:
  // Добавляет очередную вершину к ломаной линии
  Polyline &AddPoint(Point point);

  size_t GetSize() const;

  /*
   * Прочие методы и данные, необходимые для реализации элемента <polyline>
   */
 private:
  void RenderObject(const RenderContext &context) const override;

  std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
 public:
  // Задаёт координаты опорной точки (атрибуты x и y)
  Text &SetPosition(Point pos);

  // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
  Text &SetOffset(Point offset);

  // Задаёт размеры шрифта (атрибут font-size)
  Text &SetFontSize(uint32_t size);

  // Задаёт название шрифта (атрибут font-family)
  Text &SetFontFamily(std::string font_family);

  // Задаёт толщину шрифта (атрибут font-weight)
  Text &SetFontWeight(const std::string &font_weight);

  // Задаёт текстовое содержимое объекта (отображается внутри тега text)
  Text &SetData(std::string data);

  // Прочие данные и методы, необходимые для реализации элемента <text>
 private:
  void RenderObject(const RenderContext &context) const override;

  std::string GetSanitizedText(const std::string_view text) const;

  std::optional<Point> pos_ = Point(0, 0);
  std::optional<Point> offset_ = Point(0, 0);
  std::optional<uint32_t> size_ = 1;
  std::optional<std::string> font_family_;
  std::optional<std::string> font_weight_;
  std::optional<std::string> data_;
};

class ObjectContainer {
 protected:
  std::vector<std::unique_ptr<Object>> objects_ptrs_;
 public:
  template<typename Obj>
  void Add(Obj obj) {
	AddPtr(std::make_unique<Obj>(std::move(obj)));
  }

  virtual void AddPtr(std::unique_ptr<Object> &&) = 0;
};

class Document : public ObjectContainer {
 public:
  Document();
  Document(RenderContext ctx);

  // Добавляет в svg-документ объект-наследник svg::Object
  void AddPtr(std::unique_ptr<Object> &&obj) override;

  // Выводит в ostream svg-представление документа
  void Render(std::ostream &out) const;

  // Прочие методы и данные, необходимые для реализации класса Document
 private:
  RenderContext ctx_;
};

class Drawable {
 public:
  virtual void Draw(ObjectContainer &object_container) const = 0;
  virtual ~Drawable() = default;
};

}  // namespace svg