#define _USE_MATH_DEFINES
#include <cmath>
#include "svg.h"

svg::Polyline CreateStar(svg::Point center, double outer_rad, double inner_rad, int num_rays) {
  using namespace svg;
  Polyline polyline;
  for (int i = 0; i <= num_rays; ++i) {
	double angle = 2 * M_PI * (i % num_rays) / num_rays;
	polyline.AddPoint({center.x + outer_rad * sin(angle), center.y - outer_rad * cos(angle)});
	if (i == num_rays) {
	  break;
	}
	angle += M_PI / num_rays;
	polyline.AddPoint({center.x + inner_rad * sin(angle), center.y - inner_rad * cos(angle)});
  }
  return polyline;
}

using namespace std::literals;
using namespace svg;

namespace shapes {

class Triangle : public svg::Drawable {
 public:
  Triangle(svg::Point p1, svg::Point p2, svg::Point p3)
	  : p1_(p1), p2_(p2), p3_(p3) {
  }

  // Реализует метод Draw интерфейса svg::Drawable
  void Draw(svg::ObjectContainer &container) const override {
	container.Add(svg::Polyline().AddPoint(p1_).AddPoint(p2_).AddPoint(p3_).AddPoint(p1_));
  }

 private:
  svg::Point p1_, p2_, p3_;
};

class Star : public Drawable {
  svg::Point center_;
  double outer_radius_;
  double inner_radius_;
  int num_rays_;
 public:
  Star(svg::Point center, double outer_radius, double inner_radius, int num_rays)
	:
	center_(center),
	outer_radius_(outer_radius),
	inner_radius_(inner_radius),
	num_rays_(num_rays) {}

  void Draw(svg::ObjectContainer &container) const override {
	container.Add(
		CreateStar(center_, outer_radius_, inner_radius_, num_rays_)
		.SetFillColor("red"s)
		.SetStrokeColor("black"s)
		);
  }
};

class Snowman : public Drawable {
  svg::Point head_center_;
  double head_radius_;
 public:
  Snowman(svg::Point head_center, double head_radius) : head_center_(head_center), head_radius_(head_radius) {}

  void Draw(svg::ObjectContainer &container) const override {
	container.Add(
	  Circle()
	    .SetCenter({ head_center_.x, head_center_.y + 5 * head_radius_ })
		.SetRadius(head_radius_ * 2)
		.SetFillColor("rgb(240,240,240)"s)
		.SetStrokeColor("black"s)
	);
	container.Add(Circle().SetCenter({ head_center_.x, head_center_.y + 2 * head_radius_ }).SetRadius(head_radius_ * 1.5).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
	container.Add(Circle().SetCenter(head_center_).SetRadius(head_radius_).SetFillColor("rgb(240,240,240)"s).SetStrokeColor("black"s));
  }
};

} // namespace shapes

// Выполняет линейную интерполяцию значения от from до to в зависимости от параметра t.
uint8_t Lerp(uint8_t from, uint8_t to, double t) {
  return static_cast<uint8_t>(std::round((to - from) * t + from));
}

int main() {
  using namespace svg;
  using namespace std;

  const uint8_t start_r = 0;
  const uint8_t end_r = 20;
  const uint8_t start_g = 255;
  const uint8_t end_g = 20;
  const uint8_t start_b = 30;
  const uint8_t end_b = 150;

  const int num_circles = 10;
  Document doc;
  for (int i = 0; i < num_circles; ++i) {
	const double t = double(i) / (num_circles - 1);

	const string r = to_string(Lerp(start_r, end_r, t));
	const string g = to_string(Lerp(start_g, end_g, t));
	const string b = to_string(Lerp(start_b, end_b, t));

	string fill_color = "rgb("s + r + ","s + g + ","s + b + ")"s;
	doc.Add(Circle()
				.SetFillColor(fill_color)
				.SetStrokeColor("black"s)
				.SetCenter({i * 20.0 + 40, 40.0})
				.SetRadius(15));
  }
  doc.Render(cout);
}
