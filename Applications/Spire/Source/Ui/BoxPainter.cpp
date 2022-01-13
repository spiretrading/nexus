#include "Spire/Ui/BoxPainter.hpp"
#include <QPainter>
#include <QPainterPath>

using namespace Spire;

namespace {
  void draw_background(QPainter& painter, QPainterPath& path,
      QColor background_color, const BoxPainter::Borders& borders, QSize size) {
    path.moveTo(0, borders.m_top_left_radius);
    path.arcTo(QRect(QPoint(0, 0), 2 *
      QSize(borders.m_top_left_radius, borders.m_top_left_radius)), 180, -90);
    path.lineTo(QPoint(size.width() - borders.m_top_right_radius, 0));
    path.arcTo(QRect(QPoint(size.width() - 2 * borders.m_top_right_radius, 0),
      2 * QSize(borders.m_top_right_radius, borders.m_top_right_radius)), 90,
      -90);
    path.lineTo(
      QPoint(size.width(), size.height() - borders.m_bottom_right_radius));
    path.arcTo(QRect(QPoint(size.width() - 2 * borders.m_bottom_right_radius,
      size.height() - 2 * borders.m_bottom_right_radius),
      2 * QSize(borders.m_bottom_right_radius, borders.m_bottom_right_radius)),
      0, -90);
    path.lineTo(QPoint(borders.m_bottom_left_radius, size.height()));
    path.arcTo(
      QRect(QPoint(0, size.height() - 2 * borders.m_bottom_left_radius),
      2 * QSize(borders.m_bottom_left_radius, borders.m_bottom_left_radius)),
      -90, -90);
    painter.fillPath(path, background_color);
    path.clear();
  }

  void draw_corner(QPainter& painter, QPainterPath& path,
      BoxPainter::Border horizontal_border, BoxPainter::Border vertical_border,
      int radius, QPoint origin, qreal orientation) {
    painter.save();
    painter.translate(origin);
    painter.rotate(orientation);
    auto corner_angle = -90 * (static_cast<double>(horizontal_border.m_size) /
      (vertical_border.m_size + horizontal_border.m_size));
    if(radius <= vertical_border.m_size) {
      path.moveTo(QPoint(0, vertical_border.m_size));
      path.lineTo(QPoint(0, radius - 1));
      path.arcTo(
        QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180, corner_angle);
      path.lineTo(QPoint(horizontal_border.m_size, vertical_border.m_size));
      painter.fillPath(path, horizontal_border.m_color);
    } else {
      path.moveTo(QPoint(0, radius - 1));
      path.arcMoveTo(
        QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180 + corner_angle);
    }
    auto outer_joint = [&] {
      auto currentPosition = path.currentPosition();
      return QPoint(currentPosition.x(), currentPosition.y());
    }();
    path.clear();
    if(radius <= vertical_border.m_size) {
      path.moveTo(QPoint(horizontal_border.m_size, vertical_border.m_size));
      path.lineTo(QPoint(horizontal_border.m_size, 0));
      path.lineTo(QPoint(radius, 0));
      path.arcTo(
        QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 90, 90 + corner_angle);
    } else {
      path.moveTo(QPoint(0, radius - 1));
      path.lineTo(QPoint(horizontal_border.m_size, radius - 1));
      path.arcTo(QRect(QPoint(horizontal_border.m_size, vertical_border.m_size),
        2 * QSize(radius - horizontal_border.m_size, radius -
          vertical_border.m_size)), 180, corner_angle);
      path.lineTo(outer_joint);
      path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)),
        180 + corner_angle, -corner_angle);
      painter.fillPath(path, horizontal_border.m_color);
      path.clear();
      path.moveTo(outer_joint);
      path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)),
        180 + corner_angle, -90 - corner_angle);
      path.lineTo(QPoint(radius, vertical_border.m_size));
      path.arcTo(QRect(QPoint(horizontal_border.m_size, vertical_border.m_size),
        2 * QSize(radius - horizontal_border.m_size, radius -
          vertical_border.m_size)), 90, 90 + corner_angle);
    }
    painter.fillPath(path, vertical_border.m_color);
    painter.restore();
    path.clear();
  }
}

BoxPainter::BoxPainter()
  : m_background_color(Qt::transparent),
    m_borders({Border(0, Qt::transparent), Border(0, Qt::transparent),
      Border(0, Qt::transparent), Border(0, Qt::transparent), 0, 0, 0, 0}) {}

BoxPainter::BoxPainter(QColor background_color, Borders borders)
  : m_background_color(background_color),
    m_borders(borders) {}

QColor BoxPainter::get_background_color() const {
  return m_background_color;
}

void BoxPainter::set_background_color(QColor color) {
  m_background_color = color;
}

const BoxPainter::Borders& BoxPainter::get_borders() const {
  return m_borders;
}

void BoxPainter::set_borders(const Borders& borders) {
  m_borders = borders;
}

void BoxPainter::paint(QPainter& painter) const {
  auto path = QPainterPath();
  auto size = QSize(painter.device()->width(), painter.device()->height());
  draw_background(painter, path, m_background_color, m_borders, size);
  painter.fillRect(QRect(QPoint(0, m_borders.m_top_left_radius - 1), QSize(
    m_borders.m_left.m_size, size.height() - m_borders.m_top_left_radius -
    m_borders.m_bottom_left_radius + 1)), m_borders.m_left.m_color);
  painter.fillRect(QRect(QPoint(m_borders.m_top_left_radius, 0), QSize(
    size.width() - m_borders.m_top_left_radius - m_borders.m_top_right_radius +
    1, m_borders.m_top.m_size)), m_borders.m_top.m_color);
  painter.fillRect(QRect(QPoint(
    size.width() - m_borders.m_right.m_size, m_borders.m_top_right_radius - 1),
    QSize(m_borders.m_right.m_size, size.height() -
      m_borders.m_top_right_radius - m_borders.m_bottom_right_radius + 2)),
      m_borders.m_right.m_color);
  painter.fillRect(QRect(QPoint(m_borders.m_bottom_left_radius - 1,
    size.height() - m_borders.m_bottom.m_size), QSize(
    size.width() - m_borders.m_bottom_left_radius -
    m_borders.m_bottom_right_radius + 1, m_borders.m_bottom.m_size)),
    m_borders.m_bottom.m_color);
  draw_corner(painter, path, m_borders.m_left, m_borders.m_top,
    m_borders.m_top_left_radius, QPoint(0, 0), 0);
  draw_corner(painter, path, m_borders.m_top, m_borders.m_right,
    m_borders.m_top_right_radius, QPoint(size.width(), 0), 90);
  draw_corner(painter, path, m_borders.m_right, m_borders.m_bottom,
    m_borders.m_bottom_right_radius, QPoint(size.width(), size.height()), 180);
  draw_corner(painter, path, m_borders.m_bottom, m_borders.m_left,
    m_borders.m_bottom_left_radius, QPoint(0, size.height()), 270);
}
