#include <numbers>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QPainterPath>
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
    Border horizontal_border, Border vertical_border, int radius) {
  path.moveTo(QPoint(0, radius - 1));
  auto corner_angle = -90 * (static_cast<double>(horizontal_border.m_size) /
    (vertical_border.m_size + horizontal_border.m_size));
  path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180, corner_angle);
  auto outer_joint = path.currentPosition();
  path.lineTo(QPoint(outer_joint.x(), radius - 1));
  painter.fillPath(path, horizontal_border.m_color);
  path.clear();
  path.moveTo(QPoint(outer_joint.x(), radius - 1));
  path.lineTo(QPoint(horizontal_border.m_size, radius - 1));
  path.arcTo(QRect(QPoint(horizontal_border.m_size, vertical_border.m_size),
    2 * QSize(radius - horizontal_border.m_size, radius - vertical_border.m_size)),
    180, corner_angle);
  auto inner_joint = path.currentPosition();
  path.lineTo(outer_joint);
  painter.fillPath(path, horizontal_border.m_color);
  path.clear();
  path.moveTo(outer_joint);
  path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180 + corner_angle,
    -90 - corner_angle);
  path.lineTo(QPoint(radius, vertical_border.m_size));
  path.arcTo(QRect(QPoint(horizontal_border.m_size, vertical_border.m_size),
    2 * QSize(radius - horizontal_border.m_size, radius - vertical_border.m_size)),
    90, 90 + corner_angle);
  painter.fillPath(path, vertical_border.m_color);
}

void draw_border(QSize size, Borders borders, QWidget& widget) {
  auto painter = QPainter(&widget);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.fillRect(QRect(
    QPoint(borders.m_left.m_size, borders.m_top.m_size),
    size - QSize(borders.m_left.m_size + borders.m_right.m_size,
      borders.m_top.m_size - borders.m_bottom.m_size)), Qt::white);
  painter.fillRect(QRect(QPoint(0, borders.m_top_left_radius - 1), QSize(
    borders.m_left.m_size, size.height() - borders.m_top_left_radius + 1)),
    borders.m_left.m_color);
  auto path = QPainterPath();
  draw_corner(
    painter, path, borders.m_left, borders.m_top, borders.m_top_left_radius);
  painter.fillRect(
    QRect(QPoint(borders.m_top_left_radius, 0),
    QSize(size.width() - borders.m_top_left_radius, borders.m_top.m_size)),
    borders.m_top.m_color);
}

struct Canvas : QWidget {
  void paintEvent(QPaintEvent* event) override {
    auto borders = Borders();
    borders.m_left = {scale_width(20), Qt::blue};
    borders.m_top = {scale_height(20), Qt::red};
    borders.m_top_left_radius = scale_width(30);
    draw_border(size(), borders, *this);
  };
};

int main(int argc, char** argv) {
  auto application = new QApplication(argc, argv);
  application->setOrganizationName(QObject::tr("Spire Trading Inc"));
  application->setApplicationName(QObject::tr("Scratch"));
  initialize_resources();
  auto canvas = new Canvas();
  canvas->resize(scale(1000, 1000));
  canvas->show();
  application->exec();
}
