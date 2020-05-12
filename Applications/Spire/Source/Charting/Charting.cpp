#include "Spire/Charting/Charting.hpp"
#include <algorithm>

using namespace Spire;

double Spire::slope(const QPointF& first, const QPointF& second) {
  return (second.y() - first.y()) / (second.x() - first.x());
}

double Spire::distance_squared(const QPointF& first, const QPointF& second) {
  return (second.x() - first.x()) * (second.x() - first.x()) +
    (second.y() - first.y()) * (second.y() - first.y());
}

bool Spire::is_within_interval(double value, double start, double end,
    double threshold) {
  if(start > end) {
    return is_within_interval(value, end, start, threshold);
  }
  return start - threshold <= value && value <= end + threshold;
}

bool Spire::is_within_interval(double value, double start, double end) {
  return is_within_interval(value, start, end, 0);
}

double Spire::y_intercept(const QPointF& point, double slope) {
  if(std::isinf(slope)) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return point.y() - point.x() * slope;
}

double Spire::calculate_y(double m, double x, double b) {
  return m * x + b;
}

double Spire::closest_point_distance_squared(QPointF point,
    const QPointF& first, const QPointF& second) {
  return std::min(distance_squared(point, first),
    distance_squared(point, second));
}
