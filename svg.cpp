#include "svg.h"

namespace svg {

using namespace std;
using namespace literals;

std::ostream &operator<<(std::ostream &os, Color color) {
  return os << visit(ColorStringifier(), color);
}

bool operator != (Color lhs, const std::string &rhs) {
  return visit(ColorStringifier(), lhs) != rhs;
}

bool operator != (Color lhs, std::string &rhs) {
  return lhs != static_cast<const string &>(rhs);
}

std::string ColorStringifier::operator()(std::monostate) {
  return "none"s;
}
std::string ColorStringifier::operator()(Rgb &clr) {
  string rslt;
  rslt.reserve(17);

  rslt += "rgb(";
  rslt += std::to_string(clr.red);
  rslt += ",";
  rslt += std::to_string(clr.green);
  rslt += ",";
  rslt += std::to_string(clr.blue);
  rslt += ")";

  return rslt;
}
std::string ColorStringifier::operator()(Rgba & clr) {
  string rslt;
  rslt.reserve(21);

  rslt += "rgba(";
  rslt += std::to_string(clr.red);
  rslt += ",";
  rslt += std::to_string(clr.green);
  rslt += ",";
  rslt += std::to_string(clr.blue);
  rslt += ",";
  rslt += std::to_string(clr.opacity);
  rslt += ")";

  return rslt;
}
std::string ColorStringifier::operator()(std::string &str) {
  return str;
}

std::unordered_map<StrokeLineCap, std::string> strlncp_to_key = {
	{StrokeLineCap::BUTT, "butt"},
	{StrokeLineCap::ROUND, "round"},
	{StrokeLineCap::SQUARE, "square"},
};

std::unordered_map<StrokeLineJoin, std::string> strlnjn_to_key = {
	{StrokeLineJoin::ARCS, "arcs"},
	{StrokeLineJoin::BEVEL, "bevel"},
	{StrokeLineJoin::MITER, "miter"},
	{StrokeLineJoin::MITER_CLIP, "miter-clip"},
	{StrokeLineJoin::ROUND, "round"},
};

std::ostream &operator<<(std::ostream &os, StrokeLineCap cap) {
  if (!strlncp_to_key.count(cap)) throw std::runtime_error("Not valid cap");

  return os << strlncp_to_key[cap];
}

std::ostream &operator<<(std::ostream &os, StrokeLineJoin join) {
  if (!strlnjn_to_key.count(join)) throw std::runtime_error("Not valid join");

  return os << strlnjn_to_key[join];
}

void Object::Render(const RenderContext &context) const {
  context.RenderIndent();

  // Делегируем вывод тега своим подклассам
  RenderObject(context);

  context.out << endl;
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

  out << "<circle"sv;
  RenderAttrs(out);
  out << " cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
  out << "r=\""sv << radius_ << "\" "sv;
  out << "/>"sv;
}

Polyline &Polyline::AddPoint(Point point) {
  points_.push_back(move(point));

  return *this;
}
size_t Polyline::GetSize() const {
  return points_.size();
}
void Polyline::RenderObject(const RenderContext &context) const {
  auto &out = context.out;
  out << "<polyline";

  RenderAttrs(out);

  out << " points=\"";
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
Text &Text::SetFontFamily(string font_family) {
  font_family_ = font_family;

  return *this;
}
Text &Text::SetFontWeight(const string &font_weight) {
  font_weight_ = font_weight;

  return *this;
}
Text &Text::SetData(string data) {
  data_ = move(data);

  return *this;
}
void Text::RenderObject(const RenderContext &context) const {
  auto &out = context.out;

  out << "<text"s;

  RenderAttrs(out);

  if (pos_.has_value()) {
	out << " x=\"" << pos_.value().x << "\""s;
	out << " y=\"" << pos_.value().y << "\""s;
  }
  if (offset_.has_value()) {
	out << " dx=\"" << offset_.value().x << "\""s;
	out << " dy=\"" << offset_.value().y << "\""s;
  }
  out << " font-size=\"" << size_.value() << "\""s;
  if (font_family_.has_value()) out << " font-family=\"" << font_family_.value() << "\""s;
  if (font_weight_.has_value()) out << " font-weight=\"" << font_weight_.value() << "\""s;

  out << ">";
  if (data_.has_value()) {
	out << GetSanitizedText(data_.value());
  }
  out << "</text>";
}
string Text::GetSanitizedText(const string_view text) const {
  string rslt{};
  rslt.reserve(text.size());

  for (char c: text) {
	if (c == '"') {
	  rslt += "&quot;"s;
	  continue;
	}

	if (c == '\'') {
	  rslt += "&apos;"s;
	  continue;
	}

	if (c == '&') {
	  rslt += "&amp;"s;
	  continue;
	}

	if (c == '<') {
	  rslt += "&lt;"s;
	  continue;
	}

	if (c == '>') {
	  rslt += "&gt;"s;
	  continue;
	}

	rslt += c;
  }

  return rslt;
}

Document::Document() = default;
Document::Document(RenderContext ctx) : ctx_(ctx) {}

void Document::AddPtr(unique_ptr<Object> &&obj) {
  objects_ptrs_.push_back(move(obj));
}
void Document::Render(ostream &out) const {
  RenderContext render_context(out);

  out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << endl;
  out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << endl;

  for (auto &obj_ptr: objects_ptrs_) {
	obj_ptr->Render(render_context);
  }

  out << "</svg>"sv;
}
}  // namespace svg