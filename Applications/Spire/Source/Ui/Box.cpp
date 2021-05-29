#include "Spire/Ui/Box.hpp"
#include <QHBoxLayout>
#include <QResizeEvent>
#include "Spire/Spire/Dimensions.hpp"

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

Box::Box(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_styles([=] { commit_style(); }) {
  setObjectName("Box");
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
  connect_style_signal(*this, [=] { on_style(); });
}

void Box::resizeEvent(QResizeEvent* event) {
  if(m_body) {
    m_body_geometry = QRect(0, 0, width(), height());
    m_styles.buffer([&] {
      auto& stylist = find_stylist(*this);
      auto properties = stylist.compute_style();
      for(auto& property : properties) {
        property.visit(
          [&] (const BorderTopSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setTop(m_body_geometry.top() + size);
            });
          },
          [&] (const BorderRightSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setRight(m_body_geometry.right() - size);
            });
          },
          [&] (const BorderBottomSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setBottom(m_body_geometry.bottom() - size);
            });
          },
          [&] (const BorderLeftSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setLeft(m_body_geometry.left() + size);
            });
          },
          [&] (const PaddingTop& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setTop(m_body_geometry.top() + size);
            });
          },
          [&] (const PaddingRight& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setRight(m_body_geometry.right() - size);
            });
          },
          [&] (const PaddingBottom& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setBottom(m_body_geometry.bottom() - size);
            });
          },
          [&] (const PaddingLeft& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_body_geometry.setLeft(m_body_geometry.left() + size);
            });
          });
      }
    });
  }
  QWidget::resizeEvent(event);
}

void Box::commit_style() {
  auto stylesheet = QString(
    R"(#Box {
        border-style: solid;)");
  m_styles.write(stylesheet);
  if(stylesheet != styleSheet()) {
    setStyleSheet(stylesheet);
    style()->unpolish(this);
    style()->polish(this);
  }
  if(m_body) {
    m_container->setGeometry(m_body_geometry);
  }
}

void Box::on_style() {
  m_body_geometry = QRect(0, 0, width(), height());
  m_styles.clear();
  m_styles.buffer([&] {
    auto& stylist = find_stylist(*this);
    auto properties = stylist.compute_style();
    for(auto& property : properties) {
      property.visit(
        [&] (const BackgroundColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_styles.set("background-color", color);
          });
        },
        [&] (const BorderTopSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("border-top-width", size);
            m_body_geometry.setTop(m_body_geometry.top() + size);
          });
        },
        [&] (const BorderRightSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("border-right-width", size);
            m_body_geometry.setRight(m_body_geometry.right() - size);
          });
        },
        [&] (const BorderBottomSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("border-bottom-width", size);
            m_body_geometry.setBottom(m_body_geometry.bottom() - size);
          });
        },
        [&] (const BorderLeftSize& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("border-left-width", size);
            m_body_geometry.setLeft(m_body_geometry.left() + size);
          });
        },
        [&] (const BorderTopColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_styles.set("border-top-color", color);
          });
        },
        [&] (const BorderRightColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_styles.set("border-right-color", color);
          });
        },
        [&] (const BorderBottomColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_styles.set("border-bottom-color", color);
          });
        },
        [&] (const BorderLeftColor& color) {
          stylist.evaluate(color, [=] (auto color) {
            m_styles.set("border-left-color", color);
          });
        },
        [&] (const BorderTopLeftRadius& radius) {
          stylist.evaluate(radius, [=] (auto radius) {
            m_styles.set("border-top-left-radius", radius);
          });
        },
        [&] (const BorderTopRightRadius& radius) {
          stylist.evaluate(radius, [=] (auto radius) {
            m_styles.set("border-top-right-radius", radius);
          });
        },
        [&] (const BorderBottomRightRadius& radius) {
          stylist.evaluate(radius, [=] (auto radius) {
            m_styles.set("border-bottom-right-radius", radius);
          });
        },
        [&] (const BorderBottomLeftRadius& radius) {
          stylist.evaluate(radius, [=] (auto radius) {
            m_styles.set("border-bottom-left-radius", radius);
          });
        },
        [&] (const PaddingTop& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("padding-top", size);
            m_body_geometry.setTop(m_body_geometry.top() + size);
          });
        },
        [&] (const PaddingRight& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("padding-right", size);
            m_body_geometry.setRight(m_body_geometry.right() - size);
          });
        },
        [&] (const PaddingBottom& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("padding-bottom", size);
            m_body_geometry.setBottom(m_body_geometry.bottom() - size);
          });
        },
        [&] (const PaddingLeft& size) {
          stylist.evaluate(size, [=] (auto size) {
            m_styles.set("padding-left", size);
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
              }
            }
          });
        });
    }
  });
}
