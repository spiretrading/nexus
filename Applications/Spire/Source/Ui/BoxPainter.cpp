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
    painter.setPen(background_color);
    painter.setBrush(background_color);
    painter.drawPath(path);
    path.clear();
  }

  void draw_border(QPainter& painter, BoxPainter::Border border, int top_radius,
      BoxPainter::Border top_border, int bottom_radius,
      BoxPainter::Border bottom_border, QPoint origin, qreal orientation) {
    painter.save();
    painter.translate(origin);
    painter.rotate(orientation);
    auto size = [&] {
      if(orientation == 0 || orientation == 180) {
        return painter.device()->height();
      }
      return painter.device()->width();
    }();
    auto top = std::max(top_radius, top_border.m_size);
    auto bottom = std::max(bottom_radius, bottom_border.m_size);
    painter.setPen(border.m_color);
    painter.setBrush(border.m_color);
    painter.drawRect(QRect(
      QPoint(0, top), QSize(border.m_size, size - top - bottom) - QSize(1, 1)));
    painter.restore();
  }

  void draw_corner(QPainter& painter, QPainterPath& path,
      BoxPainter::Border horizontal_border, BoxPainter::Border vertical_border,
      int radius, QPoint origin, qreal orientation) {
    painter.save();
    painter.translate(origin);
    painter.rotate(orientation);
    auto pixmap = QPixmap(std::max(radius, horizontal_border.m_size),
      std::max(radius, vertical_border.m_size));
    pixmap.fill(Qt::transparent);
    auto pixmap_painter = QPainter(&pixmap);
    pixmap_painter.setRenderHint(QPainter::Antialiasing);
    pixmap_painter.setRenderHint(QPainter::HighQualityAntialiasing);
    auto corner_angle = -90 * (static_cast<double>(horizontal_border.m_size) /
      (vertical_border.m_size + horizontal_border.m_size));
    if(radius < vertical_border.m_size) {
      path.moveTo(QPoint(0, vertical_border.m_size - 1));
      path.lineTo(QPoint(0, radius));
    }
    if(radius <= horizontal_border.m_size || radius <= vertical_border.m_size) {
      path.arcTo(
        QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 180, corner_angle);
      path.lineTo(
        QPoint(horizontal_border.m_size - 1, vertical_border.m_size - 1));
      if(vertical_border.m_size < radius) {
        path.lineTo(QPoint(horizontal_border.m_size - 1, radius));
      }
      path.closeSubpath();
      pixmap_painter.setPen(horizontal_border.m_color);
      pixmap_painter.setBrush(horizontal_border.m_color);
      pixmap_painter.drawPath(path);
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
    if(radius <= horizontal_border.m_size || radius <= vertical_border.m_size) {
      path.moveTo(QPoint(radius, 0));
      path.arcTo(
        QRect(QPoint(0, 0), 2 * QSize(radius, radius)), 90, 90 + corner_angle);
      path.lineTo(
        QPoint(horizontal_border.m_size - 1, vertical_border.m_size - 1));
      if(horizontal_border.m_size < radius) {
        path.lineTo(QPoint(radius, vertical_border.m_size - 1));
        path.lineTo(QPoint(radius, 0));
      } else {
        path.lineTo(QPoint(horizontal_border.m_size - 1, 0));
      }
    } else {
      path.moveTo(QPoint(0, radius - 1));
      path.lineTo(QPoint(horizontal_border.m_size - 1, radius - 1));
      path.arcTo(
        QRect(QPoint(horizontal_border.m_size - 1, vertical_border.m_size - 1),
          2 * QSize(radius - horizontal_border.m_size - 1, radius -
            vertical_border.m_size - 1)), 180, corner_angle);
      path.lineTo(outer_joint);
      path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)),
        180 + corner_angle, -corner_angle);
      pixmap_painter.setPen(horizontal_border.m_color);
      pixmap_painter.setBrush(horizontal_border.m_color);
      pixmap_painter.drawPath(path);
      path.clear();
      path.moveTo(outer_joint);
      path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)),
        180 + corner_angle, -90 - corner_angle);
      path.lineTo(QPoint(radius - 1, vertical_border.m_size - 1));
      path.arcTo(QRect(QPoint(horizontal_border.m_size - 1, vertical_border.m_size - 1),
        2 * QSize(radius - horizontal_border.m_size - 1, radius -
          vertical_border.m_size - 1)), 90, 90 + corner_angle);
    }
    path.closeSubpath();
    pixmap_painter.setPen(vertical_border.m_color);
    pixmap_painter.setBrush(vertical_border.m_color);
    pixmap_painter.drawPath(path);
    painter.drawPixmap(0, 0, pixmap);
    painter.restore();
    path.clear();
  }
}

BoxPainter::BoxPainter()
  : m_background_color(Qt::transparent),
    m_borders({Border(0, Qt::transparent), Border(0, Qt::transparent),
      Border(0, Qt::transparent), Border(0, Qt::transparent), 0, 0, 0, 0}),
    m_classification(Classification::REGULAR) {}

BoxPainter::BoxPainter(QColor background_color, Borders borders)
  : m_background_color(background_color),
    m_borders(borders),
    m_classification(evaluate_classification()) {}

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
  m_classification = evaluate_classification();
}

void BoxPainter::set_top_border_size(int size) {
  m_borders.m_top.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_top_border_color(QColor color) {
  m_borders.m_top.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_right_border_size(int size) {
  m_borders.m_right.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_right_border_color(QColor color) {
  m_borders.m_right.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_bottom_border_size(int size) {
  m_borders.m_bottom.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_bottom_border_color(QColor color) {
  m_borders.m_bottom.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_left_border_size(int size) {
  m_borders.m_left.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_left_border_color(QColor color) {
  m_borders.m_left.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_top_right_radius(int radius) {
  m_borders.m_top_right_radius = radius;
  m_classification = evaluate_classification();
}

void BoxPainter::set_bottom_right_radius(int radius) {
  m_borders.m_bottom_right_radius = radius;
  m_classification = evaluate_classification();
}

void BoxPainter::set_bottom_left_radius(int radius) {
  m_borders.m_bottom_left_radius = radius;
  m_classification = evaluate_classification();
}

void BoxPainter::set_top_left_radius(int radius) {
  m_borders.m_top_left_radius = radius;
  m_classification = evaluate_classification();
}

void BoxPainter::paint(QPainter& painter) const {
  auto size = QSize(painter.device()->width(), painter.device()->height());
  if(m_classification == Classification::REGULAR) {
    painter.fillRect(QRect(QPoint(0, 0), size), m_background_color);
    painter.setPen(QPen(
      QBrush(m_borders.m_top.m_color), m_borders.m_top.m_size, Qt::SolidLine));
    painter.drawRect(QRect(QPoint(0, 0),
      size - QSize(m_borders.m_top.m_size, m_borders.m_top.m_size)));
  } else if(m_classification == Classification::REGULAR_CURVED) {
    painter.setPen(m_background_color);
    painter.setBrush(m_background_color);
    painter.drawRoundedRect(QRect(QPoint(0, 0), size),
      m_borders.m_top_left_radius, m_borders.m_top_right_radius);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(
      QBrush(m_borders.m_top.m_color), m_borders.m_top.m_size, Qt::SolidLine));
    painter.drawRoundedRect(QRect(QPoint(0, 0), size - QSize(m_borders.m_top.m_size, m_borders.m_top.m_size)),
      m_borders.m_top_left_radius, m_borders.m_top_right_radius);
  } else {
    auto path = QPainterPath();
    draw_background(painter, path, m_background_color, m_borders, size);
    draw_corner(painter, path, m_borders.m_left, m_borders.m_top,
      m_borders.m_top_left_radius, QPoint(0, 0), 0);
    draw_corner(painter, path, m_borders.m_top, m_borders.m_right,
      m_borders.m_top_right_radius, QPoint(size.width(), 0), 90);
    draw_corner(painter, path, m_borders.m_right, m_borders.m_bottom,
      m_borders.m_bottom_right_radius,
      QPoint(size.width(), size.height()), 180);
    draw_corner(painter, path, m_borders.m_bottom, m_borders.m_left,
      m_borders.m_bottom_left_radius, QPoint(0, size.height()), 270);
    draw_border(painter, m_borders.m_left, m_borders.m_top_left_radius,
      m_borders.m_top, m_borders.m_bottom_left_radius, m_borders.m_bottom,
      QPoint(0, 0), 0);
    draw_border(painter, m_borders.m_top, m_borders.m_top_right_radius,
      m_borders.m_right, m_borders.m_top_left_radius, m_borders.m_left,
      QPoint(size.width() - 1, 0), 90);
    draw_border(painter, m_borders.m_right, m_borders.m_bottom_right_radius,
      m_borders.m_bottom, m_borders.m_top_right_radius, m_borders.m_top,
      QPoint(size.width() - 1, size.height() - 1), 180);
    draw_border(painter, m_borders.m_bottom, m_borders.m_bottom_left_radius,
      m_borders.m_left, m_borders.m_bottom_right_radius, m_borders.m_right,
      QPoint(0, size.height() - 1), 270);
  }
}

BoxPainter::Classification BoxPainter::evaluate_classification() const {
  if(m_borders.m_top == m_borders.m_right && m_borders.m_right ==
      m_borders.m_bottom && m_borders.m_bottom == m_borders.m_left) {
    if(m_borders.m_top_right_radius == m_borders.m_bottom_right_radius &&
        m_borders.m_bottom_right_radius == m_borders.m_bottom_left_radius &&
        m_borders.m_bottom_left_radius == m_borders.m_top_left_radius) {
      if(m_borders.m_top_left_radius == 0) {
        return Classification::REGULAR;
      }
      return Classification::REGULAR_CURVED;
    }
  }
  return Classification::OTHER;
}
