#include "Spire/Ui/ScrollBox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"

using namespace Spire;
using namespace Spire::Styles;

ScrollBox::ScrollBox(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_horizontal_display_policy(DisplayPolicy::ALWAYS),
      m_vertical_display_policy(DisplayPolicy::ALWAYS),
      m_border_styles([=] { commit_border_styles(); }),
      m_padding_styles([=] { commit_padding_styles(); }) {
  setObjectName(QString("0x%1").arg(reinterpret_cast<std::intptr_t>(this)));
  auto layers = new LayeredWidget();
  m_viewport = new QWidget();
  m_viewport->setObjectName(
    QString("0x%1").arg(reinterpret_cast<std::intptr_t>(m_viewport)));
  m_body->installEventFilter(this);
  auto viewport_layout = new QHBoxLayout();
  viewport_layout->setContentsMargins({});
  auto inner_viewport = new QWidget();
  viewport_layout->addWidget(inner_viewport);
  m_body->setParent(inner_viewport);
  m_viewport->setLayout(viewport_layout);
  layers->add(m_viewport);
  m_scrollable_layer = new ScrollableLayer();
  m_scrollable_layer->get_vertical_scroll_bar().connect_position_signal(
    [=] (auto position) { on_vertical_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().connect_position_signal(
    [=] (auto position) { on_horizontal_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().installEventFilter(this);
  m_scrollable_layer->get_vertical_scroll_bar().installEventFilter(this);
  layers->add(m_scrollable_layer);
  auto layout = new QHBoxLayout();
  layout->setContentsMargins({});
  layout->addWidget(layers);
  setLayout(layout);
  setFocusPolicy(Qt::StrongFocus);
  update_ranges();
  connect_style_signal(*this, [=] { on_style(); });
}

QWidget& ScrollBox::get_body() {
  return *m_body;
}

ScrollBox::DisplayPolicy ScrollBox::get_horizontal_display_policy() const {
  return m_horizontal_display_policy;
}

void ScrollBox::set_horizontal(DisplayPolicy policy) {
  if(policy == m_horizontal_display_policy) {
    return;
  }
  m_horizontal_display_policy = policy;
  if(m_horizontal_display_policy == DisplayPolicy::NEVER) {
    m_scrollable_layer->get_horizontal_scroll_bar().hide();
  } else if(m_horizontal_display_policy == DisplayPolicy::ALWAYS) {
    m_scrollable_layer->get_horizontal_scroll_bar().show();
  }
  update_ranges();
}

ScrollBox::DisplayPolicy ScrollBox::get_vertical_display_policy() const {
  return m_vertical_display_policy;
}

void ScrollBox::set_vertical(DisplayPolicy policy) {
  if(policy == m_vertical_display_policy) {
    return;
  }
  m_vertical_display_policy = policy;
  if(m_vertical_display_policy == DisplayPolicy::NEVER) {
    m_scrollable_layer->get_vertical_scroll_bar().hide();
  } else if(m_vertical_display_policy == DisplayPolicy::ALWAYS) {
    m_scrollable_layer->get_vertical_scroll_bar().show();
  }
  update_ranges();
}

void ScrollBox::set(DisplayPolicy policy) {
  set(policy, policy);
}

void ScrollBox::set(
    DisplayPolicy horizontal_policy, DisplayPolicy vertical_policy) {
  set_horizontal(horizontal_policy);
  set_vertical(vertical_policy);
}

bool ScrollBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched != m_body) {
    if(event->type() == QEvent::Show || event->type() == QEvent::Hide) {
      update_ranges();
    }
  }
  if(event->type() == QEvent::Resize) {
    update_ranges();
  }
  return QWidget::eventFilter(watched, event);
}

void ScrollBox::keyPressEvent(QKeyEvent* event) {
  m_scrollable_layer->keyPressEvent(event);
}

void ScrollBox::resizeEvent(QResizeEvent* event) {
  update_ranges();
}

void ScrollBox::wheelEvent(QWheelEvent* event) {
  m_scrollable_layer->wheelEvent(event);
}

void ScrollBox::commit_border_styles() {
  auto stylesheet = QString(
    R"(#0x%1 {
        border-style: solid;)").arg(reinterpret_cast<std::intptr_t>(this));
  m_border_styles.write(stylesheet);
  if(m_borders != layout()->contentsMargins()) {
    layout()->setContentsMargins(m_borders);
  }
  if(stylesheet != styleSheet()) {
    setStyleSheet(stylesheet);
    style()->unpolish(this);
    style()->polish(this);
  }
}

void ScrollBox::commit_padding_styles() {
  auto stylesheet =
    QString("#0x%1 {").arg(reinterpret_cast<std::intptr_t>(m_viewport));
  m_padding_styles.write(stylesheet);
  if(m_padding != m_viewport->layout()->contentsMargins()) {
    m_viewport->layout()->setContentsMargins(m_padding);
  }
  if(stylesheet != m_viewport->styleSheet()) {
    m_viewport->setStyleSheet(stylesheet);
    m_viewport->style()->unpolish(m_viewport);
    m_viewport->style()->polish(m_viewport);
  }
}

void ScrollBox::on_style() {
  m_border_styles.clear();
  m_padding_styles.clear();
  m_borders = {};
  m_padding = {};
  m_border_styles.buffer([&] {
    m_padding_styles.buffer([&] {
      auto& stylist = find_stylist(*this);
      for(auto& property : stylist.get_computed_block()) {
        property.visit(
          [&] (const BorderTopSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_border_styles.set("border-top-width", size);
              m_borders.setTop(size);
            });
          },
          [&] (const BorderRightSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_border_styles.set("border-right-width", size);
              m_borders.setRight(size);
            });
          },
          [&] (const BorderBottomSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_border_styles.set("border-bottom-width", size);
              m_borders.setBottom(size);
            });
          },
          [&] (const BorderLeftSize& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_border_styles.set("border-left-width", size);
              m_borders.setLeft(size);
            });
          },
          [&] (const BorderTopColor& color) {
            stylist.evaluate(color, [=] (auto color) {
              m_border_styles.set("border-top-color", color);
            });
          },
          [&] (const BorderRightColor& color) {
            stylist.evaluate(color, [=] (auto color) {
              m_border_styles.set("border-right-color", color);
            });
          },
          [&] (const BorderBottomColor& color) {
            stylist.evaluate(color, [=] (auto color) {
              m_border_styles.set("border-bottom-color", color);
            });
          },
          [&] (const BorderLeftColor& color) {
            stylist.evaluate(color, [=] (auto color) {
              m_border_styles.set("border-left-color", color);
            });
          },
          [&] (const BorderTopLeftRadius& radius) {
            stylist.evaluate(radius, [=] (auto radius) {
              m_border_styles.set("border-top-left-radius", radius);
            });
          },
          [&] (const BorderTopRightRadius& radius) {
            stylist.evaluate(radius, [=] (auto radius) {
              m_border_styles.set("border-top-right-radius", radius);
            });
          },
          [&] (const BorderBottomRightRadius& radius) {
            stylist.evaluate(radius, [=] (auto radius) {
              m_border_styles.set("border-bottom-right-radius", radius);
            });
          },
          [&] (const BorderBottomLeftRadius& radius) {
            stylist.evaluate(radius, [=] (auto radius) {
              m_border_styles.set("border-bottom-left-radius", radius);
            });
          },
          [&] (const BackgroundColor& color) {
            stylist.evaluate(color, [=] (auto color) {
              m_padding_styles.set("background-color", color);
            });
          },
          [&] (const PaddingTop& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_padding_styles.set("padding-top", size);
              m_padding.setTop(size);
            });
          },
          [&] (const PaddingRight& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_padding_styles.set("padding-right", size);
              m_padding.setRight(size);
            });
          },
          [&] (const PaddingBottom& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_padding_styles.set("padding-bottom", size);
              m_padding.setBottom(size);
            });
          },
          [&] (const PaddingLeft& size) {
            stylist.evaluate(size, [=] (auto size) {
              m_padding_styles.set("padding-left", size);
              m_padding.setLeft(size);
            });
          });
      }
    });
  });
}

void ScrollBox::on_vertical_scroll(int position) {
  m_body->move(m_body->pos().x(), -position);
}

void ScrollBox::on_horizontal_scroll(int position) {
  m_body->move(-position, m_body->pos().y());
}

void ScrollBox::update_ranges() {
  auto bar_width = [&] {
    if(m_scrollable_layer->get_vertical_scroll_bar().isVisible()) {
      return m_scrollable_layer->get_vertical_scroll_bar().width();
    }
    return 0;
  }();
  auto bar_height = [&] {
    if(m_scrollable_layer->get_horizontal_scroll_bar().isVisible()) {
      return m_scrollable_layer->get_horizontal_scroll_bar().height();
    }
    return 0;
  }();
  auto viewport_size = m_body->size() + QSize(bar_width, bar_height);
  if(m_vertical_display_policy == DisplayPolicy::ON_OVERFLOW) {
    if(viewport_size.height() <= height()) {
      m_scrollable_layer->get_vertical_scroll_bar().hide();
    } else {
      m_scrollable_layer->get_vertical_scroll_bar().show();
    }
  }
  if(m_horizontal_display_policy == DisplayPolicy::ON_OVERFLOW) {
    if(viewport_size.width() <= width()) {
      m_scrollable_layer->get_horizontal_scroll_bar().hide();
    } else {
      m_scrollable_layer->get_horizontal_scroll_bar().show();
    }
  }
  auto vertical_range = std::max(m_body->height() - height() + bar_height, 0);
  auto horizontal_range = std::max(m_body->width() - width() + bar_width, 0);
  m_scrollable_layer->get_vertical_scroll_bar().set_range(0, vertical_range);
  m_scrollable_layer->get_vertical_scroll_bar().set_page_size(height());
  m_scrollable_layer->get_horizontal_scroll_bar().set_range(
    0, horizontal_range);
  m_scrollable_layer->get_horizontal_scroll_bar().set_page_size(width());
}

std::unordered_set<Stylist*> BaseComponentFinder<ScrollBox, Body>::operator ()(
    ScrollBox& box, const Body& body) const {
  auto stylists = std::unordered_set<Stylist*>();
  stylists.insert(&find_stylist(box.get_body()));
  return stylists;
}
