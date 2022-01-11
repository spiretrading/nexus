#include "Spire/Ui/Box.hpp"
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPathStroker>
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  struct QuadraticBezierCurve {
    QPointF m_start;
    QPointF m_control;
    QPointF m_end;
  };

  struct BorderCornerCurves {
    QuadraticBezierCurve m_outer;
    QuadraticBezierCurve m_inner;
  };

  struct BorderSideCurves {
    BorderCornerCurves m_left;
    BorderCornerCurves m_right;
  };

  double get_transition_point(int border_width1, int border_width2) {
    return static_cast<double>(border_width1) / (border_width1 + border_width2);
  }

  QPointF get_curve_position(
      const QuadraticBezierCurve& curve, double percent) {
    double x1 =
      (curve.m_control.x() - curve.m_start.x()) * percent + curve.m_start.x();
    double y1 =
      (curve.m_control.y() - curve.m_start.y()) * percent + curve.m_start.y();
    double x2 =
      (curve.m_end.x() - curve.m_control.x()) * percent + curve.m_control.x();
    double y2 =
      (curve.m_end.y() - curve.m_control.y()) * percent + curve.m_control.y();
    return {(x2 - x1) * percent + x1, (y2 - y1) * percent + y1};
  }

  QuadraticBezierCurve make_left_outer_border_curve(
      int radius, int border, int previous_border) {
    auto start = [&] {
      if(previous_border <= 0) {
        return QPointF(0, radius);
      }
      auto start = QPointF(0, radius);
      auto end = QPointF(radius, 0);
      auto tp = get_transition_point(border, previous_border);
      auto point = get_curve_position({start, QPointF(0, 0), end}, 1 - tp);
      return point;
    }();
    auto end = [&] {
      return QPointF(radius, 0);
    }();
    auto control = [&] {
      return QPointF(static_cast<double>(radius / 2), 0);
    }();
    return {start, control, end};
  }

  QuadraticBezierCurve make_left_inner_border_curve(
      int radius, int border_width, int previous_border_width) {
    auto start = [&] {
      if(previous_border_width <= 0) {
        return QPointF(previous_border_width + radius, border_width);
      }
      // TODO: these curves are the same for each start/end of the inner corners,
      //        so don't duplicate
      return QPointF(
        previous_border_width + std::max(0, radius - previous_border_width),
        border_width);
    }();
    auto ideal_end = QPointF(previous_border_width,
      border_width + std::max(0, radius - border_width));
    auto end = [&] {
      if(previous_border_width <= 0) {
        return QPointF(
          previous_border_width, border_width + std::max(0, radius - border_width));
      } else if(radius - previous_border_width <= 0) {
        return QPointF(previous_border_width, border_width);
      }
      auto start = QPointF(
        previous_border_width + std::max(0, radius - border_width),
        border_width);
      auto tp = get_transition_point(border_width, previous_border_width);
      auto point = get_curve_position(
        {start, QPointF(previous_border_width, border_width), ideal_end}, tp);
      return point;
    }();
    auto ctrl = [&] {
      return QPointF(end.x() + (end.x() - ideal_end.x()), border_width);
    }();
    return {start, ctrl, end};
  }

  QuadraticBezierCurve make_right_outer_border_curve(
      int radius, int border_width, int next_border_width, int widget_width) {
    auto start = [&] {
      return QPointF(widget_width - radius, 0);
    }();
    auto end = [&] {
      if(next_border_width <= 0) {
        return QPointF(widget_width, radius);
      }
      auto start = QPointF(widget_width - radius, 0);
      auto end = QPointF(widget_width, radius);
      auto tp = get_transition_point(border_width, next_border_width);
      auto point = get_curve_position(
        {start, QPointF(widget_width, 0), end}, tp);
      return point;
    }();
    auto control = [&] {
      return QPointF(widget_width - (radius / 2), 0);
    }();
    return {start, control, end};
  }

  QuadraticBezierCurve make_right_inner_border_curve(
      int radius, int border_width, int next_border_width, int widget_width) {
    auto ideal_start = QPointF(widget_width - next_border_width, radius);
    auto start = [&] {
      if(next_border_width <= 0) {
        return QPointF(widget_width, radius);
      } else if(radius - next_border_width <= 0) {
        return QPointF(widget_width - next_border_width, border_width);
      }
      auto end = QPointF(widget_width - radius, border_width);
      auto tp = get_transition_point(border_width, next_border_width);
      // TODO: address this 1 - tp calculation by having a way from inferring
      //        direction in the get_transition_point function.
      auto point = get_curve_position({ideal_start,
        QPointF(widget_width - next_border_width, border_width), end}, 1 - tp);
      return point;
    }();
    auto end = [&] {
      if(next_border_width <= 0) {
        return QPointF(widget_width - radius, border_width);
      }
      return QPointF(widget_width - (next_border_width +
        std::max(0, radius - next_border_width)), border_width);
    }();
    auto ctrl = [&] {
      return QPointF(start.x() - (ideal_start.x() - start.x()), border_width);
    }();
    return {start, ctrl, end};
  }

  BorderCornerCurves make_left_border_curves(
      int radius, int border_width, int previous_border_width) {
    return {
      make_left_outer_border_curve(
        radius, border_width, previous_border_width),
      make_left_inner_border_curve(
        radius, border_width, previous_border_width)};
  }

  BorderCornerCurves make_right_border_curves(
      int radius, int border_width, int next_border_width, int widget_width) {
    return {
      make_right_outer_border_curve(
        radius, border_width, next_border_width, widget_width),
      make_right_inner_border_curve(
        radius, border_width, next_border_width, widget_width)};
  }

  BorderSideCurves make_border_side_curves(
      int left_radius, int right_radius, int border_width,
      int previous_border_width, int next_border_width, int widget_width) {
    return {
      make_left_border_curves(
        left_radius, border_width, previous_border_width),
      make_right_border_curves(
        right_radius, border_width, next_border_width, widget_width)};
  }

  auto make_border_side_path(const BorderSideCurves& curves) {
    const auto& left_outer = curves.m_left.m_outer;
    const auto& left_inner = curves.m_left.m_inner;
    const auto& right_outer = curves.m_right.m_outer;
    const auto& right_inner = curves.m_right.m_inner;
    auto path = QPainterPath(left_outer.m_start);
    path.quadTo(left_outer.m_control, left_outer.m_end);
    path.lineTo(right_outer.m_start);
    path.quadTo(right_outer.m_control, right_outer.m_end);
    path.lineTo(right_inner.m_start);
    path.quadTo(right_inner.m_control, right_inner.m_end);
    path.lineTo(left_inner.m_start);
    path.quadTo(left_inner.m_control, left_inner.m_end);
    path.lineTo(left_outer.m_start);
    return path;
  }
}

BorderSize Spire::Styles::border_size(Expression<int> size) {
  return BorderSize(size, size, size, size);
}

BorderColor Spire::Styles::border_color(Expression<QColor> color) {
  return BorderColor(color, color, color, color);
}

BorderRadius Spire::Styles::border_radius(Expression<int> radius) {
  return BorderRadius(radius, radius, radius, radius);
}

Border Spire::Styles::border(Expression<int> size, Expression<QColor> color) {
  return Border(border_size(size), border_color(color));
}

HorizontalPadding Spire::Styles::horizontal_padding(int size) {
  return HorizontalPadding(PaddingRight(size), PaddingLeft(size));
}

VerticalPadding Spire::Styles::vertical_padding(int size) {
  return VerticalPadding(PaddingTop(size), PaddingBottom(size));
}

Padding Spire::Styles::padding(int size) {
  return Padding(PaddingTop(size), PaddingRight(size), PaddingBottom(size),
    PaddingLeft(size));
}

Box::BoxStyle::BoxStyle()
  : m_padding{0, 0, 0, 0},
    m_background_color{Qt::transparent},
    m_border_width{0, 0, 0, 0},
    m_border_color{
      Qt::transparent, Qt::transparent, Qt::transparent, Qt::transparent},
    m_border_radius{0, 0, 0, 0} {}

Box::Box(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body) {
  setObjectName(QString("0x%1").arg(reinterpret_cast<std::intptr_t>(this)));
  auto box_layout = new QHBoxLayout(this);
  box_layout->setContentsMargins({});
  if(m_body) {
    m_container = new QWidget(this);
    auto layout = new QHBoxLayout(m_container);
    layout->setContentsMargins({});
    layout->addWidget(m_body);
    layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setFocusProxy(m_body);
  } else {
    m_container = nullptr;
  }
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
}

const QWidget* Box::get_body() const {
  return m_body;
}

QWidget* Box::get_body() {
  return m_body;
}

QSize Box::sizeHint() const {
  if(m_size_hint) {
    return *m_size_hint;
  } else if(m_container) {
    m_size_hint.emplace(m_container->sizeHint());
    if(m_size_hint->isValid()) {
      for(auto& property : get_evaluated_block(*this)) {
        property.visit(
          [&] (std::in_place_type_t<BorderTopSize>, int size) {
            m_size_hint->rheight() += size;
          },
          [&] (std::in_place_type_t<BorderRightSize>, int size) {
            m_size_hint->rwidth() += size;
          },
          [&] (std::in_place_type_t<BorderBottomSize>, int size) {
            m_size_hint->rheight() += size;
          },
          [&] (std::in_place_type_t<BorderLeftSize>, int size) {
            m_size_hint->rwidth() += size;
          },
          [&] (std::in_place_type_t<PaddingTop>, int size) {
            m_size_hint->rheight() += size;
          },
          [&] (std::in_place_type_t<PaddingRight>, int size) {
            m_size_hint->rwidth() += size;
          },
          [&] (std::in_place_type_t<PaddingBottom>, int size) {
            m_size_hint->rheight() += size;
          },
          [&] (std::in_place_type_t<PaddingLeft>, int size) {
            m_size_hint->rwidth() += size;
          });
      }
    }
  } else {
    m_size_hint.emplace(QWidget::sizeHint());
  }
  return *m_size_hint;
}

bool Box::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    m_size_hint = none;
    updateGeometry();
  }
  return QWidget::event(event);
}

void Box::paintEvent(QPaintEvent* event) {
  auto painter = QPainter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.fillRect(rect(), m_style.m_background_color);
  paint_border_side(painter, m_style.m_border_width.m_top,
    m_style.m_border_color.m_top, m_border_geometry.m_top);
  paint_border_side(painter, m_style.m_border_width.m_bottom,
    m_style.m_border_color.m_bottom, m_border_geometry.m_bottom);
  paint_border_side(painter, m_style.m_border_width.m_right,
    m_style.m_border_color.m_right, m_border_geometry.m_right);
  paint_border_side(painter, m_style.m_border_width.m_left,
    m_style.m_border_color.m_left, m_border_geometry.m_left);
}

void Box::resizeEvent(QResizeEvent* event) {
  if(m_body) {
    m_body_geometry = QRect(0, 0, width(), height());
    for(auto& property : get_evaluated_block(*this)) {
      property.visit(
        [&] (std::in_place_type_t<BorderTopSize>, int size) {
          m_body_geometry.setTop(m_body_geometry.top() + size);
        },
        [&] (std::in_place_type_t<BorderRightSize>, int size) {
          m_body_geometry.setRight(m_body_geometry.right() - size);
        },
        [&] (std::in_place_type_t<BorderBottomSize>, int size) {
          m_body_geometry.setBottom(m_body_geometry.bottom() - size);
        },
        [&] (std::in_place_type_t<BorderLeftSize>, int size) {
          m_body_geometry.setLeft(m_body_geometry.left() + size);
        },
        [&] (std::in_place_type_t<PaddingTop>, int size) {
          m_body_geometry.setTop(m_body_geometry.top() + size);
        },
        [&] (std::in_place_type_t<PaddingRight>, int size) {
          m_body_geometry.setRight(m_body_geometry.right() - size);
        },
        [&] (std::in_place_type_t<PaddingBottom>, int size) {
          m_body_geometry.setBottom(m_body_geometry.bottom() - size);
        },
        [&] (std::in_place_type_t<PaddingLeft>, int size) {
          m_body_geometry.setLeft(m_body_geometry.left() + size);
        });
    }
    m_container->setGeometry(m_body_geometry);
  }
  // TODO: since the resizeEvent is called after the size has changed,
  //        will this radius update still be visible?
  //reduce_radii();
  update_border_geometry();
  QWidget::resizeEvent(event);
}

void Box::paint_border_side(QPainter& painter,
    int width, const QColor& color, const QPainterPath& geometry) const {
  if(width > 0) {
    painter.fillPath(geometry, color);
  }
}

double Box::radius_reduction_factor(const BorderRadius& radius) const {
  // TODO: instead of if, just calculate the factor then return if it's less than 1.0:
  //      if(auto factor = ...; factor < 1.0f) { return factor };
  //      or use min(..., min(..., min(...))) on each value
  if(radius.m_top_left + radius.m_top_right > width()) {
    return static_cast<double>(width()) /
      (radius.m_top_left + radius.m_top_right);
  } else if(radius.m_top_right + radius.m_bottom_right > height()) {
    return static_cast<double>(height()) /
      (radius.m_top_right + radius.m_bottom_right);
  } else if(radius.m_bottom_left + radius.m_bottom_right > width()) {
    return static_cast<double>(width()) /
      (radius.m_bottom_left + radius.m_bottom_right);
  } else if(radius.m_bottom_left + radius.m_top_left > height()) {
    return static_cast<double>(height()) /
      (radius.m_bottom_left + radius.m_bottom_right);
  }
  return 1.0f;
}

Box::BorderRadius Box::reduce_radius(BorderRadius radius) const {
  auto factor = radius_reduction_factor(radius);
  while(factor < 1.0f) {
    //qDebug() << "while factor value: " << factor;

    // Note: can't use original radii because they shouldn't be overidden in
    //        case the Box is resized later and the original radii would fit
    //        without adjustment.
    radius.m_top_left *= factor;
    radius.m_top_right *= factor;
    radius.m_bottom_right *= factor;
    radius.m_bottom_left *= factor;
    factor = radius_reduction_factor(radius);
  }
  return radius;
}

void Box::update_border_geometry() {
  auto radius = reduce_radius(m_style.m_border_radius);
  const auto& border_width = m_style.m_border_width;


  auto top_curves = make_border_side_curves(
    radius.m_top_left, radius.m_top_right, border_width.m_top,
    border_width.m_left, border_width.m_right, width());
  auto right_curves = make_border_side_curves(
    radius.m_top_right, radius.m_bottom_right, border_width.m_right,
    border_width.m_top, border_width.m_bottom, height());
  auto bottom_curves = make_border_side_curves(
    radius.m_bottom_right, radius.m_bottom_left, border_width.m_bottom,
    border_width.m_right, border_width.m_left, width());
  auto left_curves = make_border_side_curves(
    radius.m_bottom_left, radius.m_top_left, border_width.m_left,
    border_width.m_bottom, border_width.m_top, height());

  // TODO: define clip region

  m_border_geometry.m_top = make_border_side_path(top_curves);
  m_border_geometry.m_right = QTransform().
    rotate(90).translate(0, -width()).map(make_border_side_path(right_curves));
  m_border_geometry.m_bottom = QTransform().rotate(180).translate(
    -width(), -height()).map(make_border_side_path(bottom_curves));
  m_border_geometry.m_left = QTransform().rotate(270).translate(
    -height(), 0).map(make_border_side_path(left_curves));
  update();
}

void Box::on_style() {
  m_body_geometry = QRect(0, 0, width(), height());
  m_style = BoxStyle();
  auto& stylist = find_stylist(*this);
  for(auto& property : stylist.get_computed_block()) {
    property.visit(
      [&] (const BackgroundColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_style.m_background_color = color;
        });
      },
      [&] (const BorderTopSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_width.m_top = size;
          m_body_geometry.setTop(m_body_geometry.top() + size);
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_width.m_right = size;
          m_body_geometry.setRight(m_body_geometry.right() - size);
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_width.m_bottom = size;
          m_body_geometry.setBottom(m_body_geometry.bottom() - size);
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_width.m_left = size;
          m_body_geometry.setLeft(m_body_geometry.left() + size);
        });
      },
      [&] (const BorderTopColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_style.m_border_color.m_top = color;
        });
      },
      [&] (const BorderRightColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_style.m_border_color.m_right = color;
        });
      },
      [&] (const BorderBottomColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_style.m_border_color.m_bottom = color;
        });
      },
      [&] (const BorderLeftColor& color) {
        stylist.evaluate(color, [=] (auto color) {
          m_style.m_border_color.m_left = color;
        });
      },
      [&] (const BorderTopLeftRadius& radius) {
        stylist.evaluate(radius, [=] (auto radius) {
          m_style.m_border_radius.m_top_left = radius;
        });
      },
      [&] (const BorderTopRightRadius& radius) {
        stylist.evaluate(radius, [=] (auto radius) {
          m_style.m_border_radius.m_top_right = radius;
        });
      },
      [&] (const BorderBottomRightRadius& radius) {
        stylist.evaluate(radius, [=] (auto radius) {
          m_style.m_border_radius.m_bottom_right = radius;
        });
      },
      [&] (const BorderBottomLeftRadius& radius) {
        stylist.evaluate(radius, [=] (auto radius) {
          m_style.m_border_radius.m_bottom_left = radius;
        });
      },
      [&] (const PaddingTop& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_padding.m_top = size;
          m_body_geometry.setTop(m_body_geometry.top() + size);
        });
      },
      [&] (const PaddingRight& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_padding.m_right = size;
          m_body_geometry.setRight(m_body_geometry.right() - size);
        });
      },
      [&] (const PaddingBottom& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_padding.m_bottom = size;
          m_body_geometry.setBottom(m_body_geometry.bottom() - size);
        });
      },
      [&] (const PaddingLeft& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_padding.m_left = size;
          m_body_geometry.setLeft(m_body_geometry.left() + size);
        });
      },
      [&] (BodyAlign alignment) {
        stylist.evaluate(alignment, [=] (auto alignment) {
          if(m_body) {
            auto current_alignment = m_container->layout()->alignment();
            if(current_alignment != alignment) {
              m_container->layout()->setAlignment(alignment);
              m_container->layout()->update();
              m_size_hint = none;
              updateGeometry();
            }
          }
        });
      });
  }
  if(m_body) {
    m_size_hint = none;
    updateGeometry();
    m_container->setGeometry(m_body_geometry);
  }
  update_border_geometry();
}

Box* Spire::make_input_box(QWidget* body, QWidget* parent) {
  auto box = new Box(body, parent);
  auto style = StyleSheet();
  style.get(Any()).
    set(BackgroundColor(QColor(0xFFFFFF))).
    set(border(scale_width(1), QColor(0xC8C8C8))).
    set(horizontal_padding(scale_width(8))).
    set(vertical_padding(scale_height(5)));
  style.get(Hover() || FocusIn()).set(border_color(QColor(0x4B23A0)));
  style.get(Disabled()).
    set(BackgroundColor(QColor(0xF5F5F5))).
    set(border_color(QColor(0xC8C8C8)));
  style.get(ReadOnly()).
    set(BackgroundColor(QColor(Qt::transparent))).
    set(border_color(QColor(Qt::transparent))).
    set(horizontal_padding(0));
  set_style(*box, std::move(style));
  return box;
}

SelectConnection BaseComponentFinder<Box, Body>::operator ()(const Box& box,
    const Body& body, const SelectionUpdateSignal& on_update) const {
  if(auto body = box.get_body()) {
    on_update({&find_stylist(*body)}, {});
  }
  return {};
}
