#include <numbers>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QWidget>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Resources.hpp"

using namespace Spire;

struct Border {
  int m_size;
  QColor m_color;
};

struct Borders {
  Border m_top;
  Border m_right;
  Border m_bottom;
  Border m_left;
  int m_top_right_radius;
  int m_bottom_right_radius;
  int m_bottom_left_radius;
  int m_top_left_radius;
};

void draw_corner(QPainter& painter, QPainterPath& path,
    Border horizontal_border, Border vertical_border, int radius, QPoint origin,
    qreal orientation) {
  painter.save();
  painter.translate(origin);
  painter.rotate(orientation);
  path.moveTo(QPoint(0, radius - 1));
  auto corner_angle = -90 * (static_cast<double>(horizontal_border.m_size) /
    (vertical_border.m_size + horizontal_border.m_size));
  path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180, corner_angle);
  auto outer_joint = [&] {
    auto currentPosition = path.currentPosition();
    return QPoint(
      std::floor(currentPosition.x()), std::floor(currentPosition.y()));
  }();
  path.lineTo(QPoint(outer_joint.x(), radius - 1));
  painter.fillPath(path, horizontal_border.m_color);
  path.clear();
  path.moveTo(QPoint(outer_joint.x(), radius - 1));
  path.lineTo(QPoint(horizontal_border.m_size, radius - 1));
  path.arcTo(QRect(QPoint(horizontal_border.m_size, vertical_border.m_size),
    2 * QSize(radius - horizontal_border.m_size, radius -
      vertical_border.m_size)), 180, corner_angle);
  path.lineTo(outer_joint);
  painter.fillPath(path, horizontal_border.m_color);
  path.clear();
  path.moveTo(outer_joint);
  path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180 + corner_angle,
    -90 - corner_angle);
  path.lineTo(QPoint(radius, vertical_border.m_size));
  path.arcTo(QRect(QPoint(horizontal_border.m_size, vertical_border.m_size),
    2 * QSize(radius - horizontal_border.m_size, radius -
      vertical_border.m_size)), 90, 90 + corner_angle);
  painter.fillPath(path, vertical_border.m_color);
  painter.restore();
}

void draw_border(QSize size, Borders borders, QWidget& widget) {
  auto painter = QPainter(&widget);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(QRect(QPoint(borders.m_left.m_size, borders.m_top.m_size),
    size - QSize(borders.m_left.m_size + borders.m_right.m_size,
      borders.m_top.m_size + borders.m_bottom.m_size)), Qt::white);
  painter.fillRect(QRect(QPoint(0, borders.m_top_left_radius - 1), QSize(
    borders.m_left.m_size, size.height() - borders.m_top_left_radius -
    borders.m_bottom_left_radius + 1)), borders.m_left.m_color);
  painter.fillRect(QRect(QPoint(borders.m_top_left_radius, 0), QSize(
    size.width() - borders.m_top_left_radius - borders.m_top_right_radius + 1,
    borders.m_top.m_size)), borders.m_top.m_color);
  painter.fillRect(QRect(QPoint(
    size.width() - borders.m_right.m_size, borders.m_top_right_radius - 1),
    QSize(borders.m_right.m_size, size.height() - borders.m_top_right_radius -
      borders.m_bottom_right_radius + 2)), borders.m_right.m_color);
  painter.fillRect(QRect(QPoint(
    borders.m_bottom_left_radius - 1, size.height() - borders.m_bottom.m_size),
    QSize(size.width() - borders.m_bottom_left_radius -
      borders.m_bottom_right_radius + 1, borders.m_bottom.m_size)),
    borders.m_bottom.m_color);
  auto path = QPainterPath();
  draw_corner(painter, path, borders.m_left, borders.m_top,
    borders.m_top_left_radius, QPoint(0, 0), 0);
  path.clear();
  draw_corner(painter, path, borders.m_top, borders.m_right,
    borders.m_top_right_radius, QPoint(size.width(), 0), 90);
  path.clear();
  draw_corner(painter, path, borders.m_right, borders.m_bottom,
    borders.m_bottom_right_radius, QPoint(size.width(), size.height()), 180);
  path.clear();
  draw_corner(painter, path, borders.m_bottom, borders.m_left,
    borders.m_bottom_left_radius, QPoint(0, size.height()), 270);
}

struct Canvas : QWidget {
  void paintEvent(QPaintEvent* event) override {
    auto borders = Borders();
    borders.m_left = {scale_width(10), QColor(0x0000FF)};
    borders.m_top = {scale_height(20), QColor(0xFF0000)};
    borders.m_right = {scale_width(30), QColor(0x00FF00)};
    borders.m_bottom = {scale_height(40), QColor(0xFF00FF)};
    borders.m_top_left_radius = scale_width(50);
    borders.m_top_right_radius = scale_width(50);
    borders.m_bottom_right_radius = scale_width(50);
    borders.m_bottom_left_radius = scale_width(50);
    draw_border(size(), borders, *this);
  };
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto canvas = new Canvas();
  canvas->resize(scale(140, 160));
  canvas->show();
  application->exec();
}
