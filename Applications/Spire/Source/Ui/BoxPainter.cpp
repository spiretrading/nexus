#include "Spire/Ui/BoxPainter.hpp"
#include <QPainter>
#include <QPainterPath>
#include "Spire/Styles/Property.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/Box.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  void draw_borders(QPainter& painter, QPainterPath& path,
      const BoxPainter::Borders& borders, QSize size) {
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
    painter.drawPath(path);
    path.clear();
  }

  void draw_border(QPainter& painter, BoxPainter::Border border, int top_radius,
      BoxPainter::Border top_border, int bottom_radius,
      BoxPainter::Border bottom_border, QPoint origin, qreal orientation) {
    if(border.m_size <= 0) {
      return;
    }
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
    if(std::max(radius, horizontal_border.m_size) == 0 ||
        std::max(radius, vertical_border.m_size) == 0) {
      return;
    }
    painter.save();
    painter.translate(origin);
    painter.rotate(orientation);
    auto pixmap = QPixmap(std::max(radius, horizontal_border.m_size),
      std::max(radius, vertical_border.m_size));
    pixmap.fill(Qt::transparent);
    auto pixmap_painter = QPainter(&pixmap);
    pixmap_painter.setRenderHint(QPainter::Antialiasing);
    auto corner_angle = -90 * (static_cast<double>(horizontal_border.m_size) /
      std::max(1, vertical_border.m_size + horizontal_border.m_size));
    if(radius < vertical_border.m_size) {
      path.moveTo(QPoint(0, vertical_border.m_size - 1));
      path.lineTo(QPoint(0, radius));
    } else {
      path.moveTo(QPoint(0, radius));
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
          2 * QSize(radius - (horizontal_border.m_size - 1), radius -
            (vertical_border.m_size - 1))), 180, corner_angle);
      path.lineTo(outer_joint);
      path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)),
        180 + corner_angle, -corner_angle);
      if(horizontal_border.m_size > 0) {
        pixmap_painter.setPen(horizontal_border.m_color);
        pixmap_painter.setBrush(horizontal_border.m_color);
        pixmap_painter.drawPath(path);
      }
      path.clear();
      path.moveTo(outer_joint);
      path.arcTo(QRect(QPoint(0, 0), 2 * QSize(radius, radius)),
        180 + corner_angle, -90 - corner_angle);
      path.lineTo(QPoint(radius - 1, vertical_border.m_size - 1));
      path.arcTo(QRect(
        QPoint(horizontal_border.m_size - 1, vertical_border.m_size - 1),
        2 * QSize(radius - (horizontal_border.m_size - 1), radius -
          (vertical_border.m_size - 1))), 90, 90 + corner_angle);
    }
    path.closeSubpath();
    if(vertical_border.m_size > 0) {
      pixmap_painter.drawRect(
        QRect(QPoint(0, std::max(radius, vertical_border.m_size) - 1),
          QSize(horizontal_border.m_size - 1, 1)));
      pixmap_painter.setPen(vertical_border.m_color);
      pixmap_painter.setBrush(vertical_border.m_color);
      pixmap_painter.drawPath(path);
      pixmap_painter.drawRect(
        QRect(QPoint(std::max(radius, horizontal_border.m_size) - 1, 0),
          QSize(1, vertical_border.m_size - 1)));
    }
    painter.drawPixmap(0, 0, pixmap);
    painter.restore();
    path.clear();
  }
}

void Spire::apply(
    const Property& property, BoxPainter& painter, Stylist& stylist) {
  property.visit(
    [&] (const BackgroundColor& color) {
      stylist.evaluate(color, [&painter, &stylist] (auto color) {
        painter.set_background_color(color);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderTopSize& size) {
      stylist.evaluate(size, [&painter, &stylist] (auto size) {
        painter.set_border_top_size(size);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderRightSize& size) {
      stylist.evaluate(size, [&painter, &stylist] (auto size) {
        painter.set_border_right_size(size);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderBottomSize& size) {
      stylist.evaluate(size, [&painter, &stylist] (auto size) {
        painter.set_border_bottom_size(size);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderLeftSize& size) {
      stylist.evaluate(size, [&painter, &stylist] (auto size) {
        painter.set_border_left_size(size);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderTopColor& color) {
      stylist.evaluate(color, [&painter, &stylist] (auto color) {
        painter.set_border_top_color(color);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderRightColor& color) {
      stylist.evaluate(color, [&painter, &stylist] (auto color) {
        painter.set_border_right_color(color);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderBottomColor& color) {
      stylist.evaluate(color, [&painter, &stylist] (auto color) {
        painter.set_border_bottom_color(color);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderLeftColor& color) {
      stylist.evaluate(color, [&painter, &stylist] (auto color) {
        painter.set_border_left_color(color);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderTopLeftRadius& radius) {
      stylist.evaluate(radius, [&painter, &stylist] (auto radius) {
        painter.set_top_left_radius(radius);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderTopRightRadius& radius) {
      stylist.evaluate(radius, [&painter, &stylist] (auto radius) {
        painter.set_top_right_radius(radius);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderBottomRightRadius& radius) {
      stylist.evaluate(radius, [&painter, &stylist] (auto radius) {
        painter.set_bottom_right_radius(radius);
        stylist.get_widget().update();
      });
    },
    [&] (const BorderBottomLeftRadius& radius) {
      stylist.evaluate(radius, [&painter, &stylist] (auto radius) {
        painter.set_bottom_left_radius(radius);
        stylist.get_widget().update();
      });
    });
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
  m_classification = evaluate_classification();
}

const BoxPainter::Borders& BoxPainter::get_borders() const {
  return m_borders;
}

void BoxPainter::set_borders(const Borders& borders) {
  m_borders = borders;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_top_size(int size) {
  m_borders.m_top.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_top_color(QColor color) {
  m_borders.m_top.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_right_size(int size) {
  m_borders.m_right.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_right_color(QColor color) {
  m_borders.m_right.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_bottom_size(int size) {
  m_borders.m_bottom.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_bottom_color(QColor color) {
  m_borders.m_bottom.m_color = color;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_left_size(int size) {
  m_borders.m_left.m_size = size;
  m_classification = evaluate_classification();
}

void BoxPainter::set_border_left_color(QColor color) {
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
  if(m_classification == Classification::NONE) {
    return;
  }
  auto size = QSize(painter.device()->width(), painter.device()->height());
  if(m_classification == Classification::REGULAR) {
    painter.fillRect(QRect(QPoint(0, 0), size), m_background_color);
    if(m_borders.m_top.m_size > 0) {
      painter.setPen(QPen(QBrush(m_borders.m_top.m_color),
        m_borders.m_top.m_size, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
      painter.drawRect(
        QRect(QPoint(m_borders.m_top.m_size / 2, m_borders.m_top.m_size / 2),
          size - QSize(m_borders.m_top.m_size, m_borders.m_top.m_size)));
    }
  } else {
    auto path = QPainterPath();
    painter.setPen(m_background_color);
    painter.setBrush(m_background_color);
    draw_borders(painter, path, m_borders, size);
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
        if(m_borders.m_top.m_size == 0 && m_background_color.alphaF() == 0) {
          return Classification::NONE;
        }
        return Classification::REGULAR;
      }
    }
  }
  return Classification::OTHER;
}
