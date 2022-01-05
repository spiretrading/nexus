#include "Spire/Ui/Box.hpp"
#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost;
using namespace Spire;
using namespace Spire::Styles;

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
    m_background_color(Qt::transparent),
    m_border_size{0, 0, 0, 0},
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
  painter.fillRect(rect(), m_style.m_background_color);
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
  QWidget::resizeEvent(event);
}

void Box::reduce_radii() {
  throw std::exception("not implemented");
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
          m_style.m_border_size.m_top = size;
          m_body_geometry.setTop(m_body_geometry.top() + size);
        });
      },
      [&] (const BorderRightSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_size.m_right = size;
          m_body_geometry.setRight(m_body_geometry.right() - size);
        });
      },
      [&] (const BorderBottomSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_size.m_bottom = size;
          m_body_geometry.setBottom(m_body_geometry.bottom() - size);
        });
      },
      [&] (const BorderLeftSize& size) {
        stylist.evaluate(size, [=] (auto size) {
          m_style.m_border_size.m_left = size;
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
          m_style.m_border_color.m_bottom;
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
  reduce_radii();
  if(m_body) {
    m_size_hint = none;
    updateGeometry();
    m_container->setGeometry(m_body_geometry);
  }
  update();
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
