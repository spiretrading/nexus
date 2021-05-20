#include "Spire/Ui/Box.hpp"
#include <array>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QTimer>
#include <QTimeLine>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto WARNING_FRAME_COUNT = 10;

  auto get_fade_out_step(int start, int end) {
    return (end - start) / WARNING_FRAME_COUNT;
  }

  auto get_color_step(const QColor& start_color, const QColor& end_color) {
    return std::array{get_fade_out_step(start_color.red(), end_color.red()),
      get_fade_out_step(start_color.green(), end_color.green()),
      get_fade_out_step(start_color.blue(), end_color.blue())};
  }

  auto get_fade_out_color(const QColor& color, const std::array<int, 3>& steps,
      int frame) {
    return QColor(color.red() + steps[0] * frame,
      color.green() + steps[1] * frame, color.blue() + steps[2] * frame);
  }
}

BorderSize Spire::Styles::border_size(Expression<int> size) {
  return BorderSize(size, size, size, size);
}

BorderColor Spire::Styles::border_color(Expression<QColor> color) {
  return BorderColor(color, color, color, color);
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
      auto properties = stylist.compute_style().get_properties();
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
    R"(#Box {"
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
    auto properties = stylist.compute_style().get_properties();
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

void Spire::display_warning_indicator(QWidget& widget) {
  const auto WARNING_DURATION = 300;
  const auto WARNING_FADE_OUT_DELAY = 250;
  const auto WARNING_BACKGROUND_COLOR = QColor("#FFF1F1");
  const auto WARNING_BORDER_COLOR = QColor("#B71C1C");
  auto time_line = new QTimeLine(WARNING_DURATION, &widget);
  time_line->setFrameRange(0, WARNING_FRAME_COUNT);
  time_line->setEasingCurve(QEasingCurve::Linear);
  auto computed_style = compute_style(widget);
  auto computed_background_color = [&] {
    if(auto color = find<BackgroundColor>(computed_style)) {
      return color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0, 0, 0, 0);
  }();
  auto background_color_step =
    get_color_step(WARNING_BACKGROUND_COLOR, computed_background_color);
  auto computed_border_color = [&] {
    if(auto border_color = find<BorderTopColor>(computed_style)) {
      return border_color->get_expression().as<QColor>();
    }
    return QColor::fromRgb(0, 0, 0, 0);
  }();
  auto border_color_step =
    get_color_step(WARNING_BORDER_COLOR, computed_border_color);
  QObject::connect(time_line, &QTimeLine::frameChanged,
    [=, &widget] (auto frame) {
      auto frame_background_color = get_fade_out_color(WARNING_BACKGROUND_COLOR,
        background_color_step, frame);
      auto frame_border_color =
        get_fade_out_color(WARNING_BORDER_COLOR, border_color_step, frame);
      auto animated_style = get_style(widget);
      animated_style.get(Any()).
        set(BackgroundColor(frame_background_color)).
        set(border_color(frame_border_color));
      set_style(widget, std::move(animated_style));
    });
  auto style = get_style(widget);
  QObject::connect(time_line, &QTimeLine::finished, [=, &widget] () mutable {
    set_style(widget, std::move(style));
    time_line->deleteLater();
  });
  auto animated_style = StyleSheet();
  animated_style.get(Any()) = style.get(Any());
  animated_style.get(Any()).set(BackgroundColor(WARNING_BACKGROUND_COLOR));
  animated_style.get(Any()).set(border_color(WARNING_BORDER_COLOR));
  set_style(widget, std::move(animated_style));
  QTimer::singleShot(WARNING_FADE_OUT_DELAY, &widget, [=] {
    time_line->start();
  });
}
