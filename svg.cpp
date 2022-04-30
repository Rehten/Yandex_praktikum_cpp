#include "svg.h"

namespace svg {

using namespace std::literals;

void Object::Render(const RenderContext &context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
  RenderObject(context);

  context.out << std::endl;
}

// ---------- Circle ------------------

Circle &Circle::SetCenter(Point center) {
  center_ = center;
  return *this;
}

Circle &Circle::SetRadius(double radius) {
  radius_ = radius;
  return *this;
}

void Circle::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
  out << "r=\""sv << radius_ << "\" "sv;
  out << "/>"sv;
}

Polyline &Polyline::AddPoint(Point point) {
  points_.push_back(std::move(point));

  return *this;
}
size_t Polyline::GetSize() const {
  return points_.size();
}
void Polyline::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<polyline points=\"";

  for (size_t i = 0; i != points_.size(); ++i) {
	if (i) {
	  out << " ";
	}

	out << points_[i].x << "," << points_[i].y;
  }

  out << "\" />";
}
Text &Text::SetPosition(Point pos) {
  pos_ = pos;

  return *this;
}
Text &Text::SetOffset(Point offset) {
  offset_ = offset;

  return *this;
}
Text &Text::SetFontSize(uint32_t size) {
  size_ = size;

  return *this;
}
Text &Text::SetFontFamily(std::string font_family) {
  font_family_ = font_family;

  return *this;
}
Text &Text::SetFontWeight(const std::string &font_weight) {
  font_weight_ = font_weight;

  return *this;
}
Text &Text::SetData(std::string data) {
  data_ = std::move(data);

  return *this;
}
void Text::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<text x=\"" << pos_.x
	  << "\" y=\"" << pos_.y
	  << "\" dx=\"" << offset_.x
	  << "\" dy=\"" << offset_.y
	  << "\" font-size=\"" << size_
	  << "\" font-family=\"" << font_family_
	  << "\" font-weight=\"" << font_weight_
  	  << "\" >" << data_ << "</text>";
}

Document::Document() = default;
Document::Document(RenderContext ctx) : ctx_(ctx) {}

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
  objects_ptrs_.push_back(std::move(obj));
}
void Document::Render(std::ostream &out) const {
  RenderContext render_context(out);

  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;

  for (auto &obj_ptr : objects_ptrs_) {
	obj_ptr->Render(render_context);
  }

  out << "</svg>"sv;
}
}  // namespace svg