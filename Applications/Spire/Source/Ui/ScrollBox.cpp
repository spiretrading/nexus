#include "Spire/Ui/ScrollBox.hpp"
#include <QEvent>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/Box.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/Layouts.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"

using namespace boost::posix_time;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  const auto EASE_DURATION = millisec(60);

  auto NARROW_SCROLL_BAR_SIZE() {
    static auto size = scale(6, 6);
    return size;
  }

  auto WIDE_SCROLL_BAR_SIZE() {
    static auto size = scale(13, 13);
    return size;
  }

  QSize get_scroll_bar_size(ScrollableLayer& layer,
      ScrollBox::DisplayPolicy horizontal_policy,
      ScrollBox::DisplayPolicy vertical_policy) {
    auto width = [&] {
      if(vertical_policy != ScrollBox::DisplayPolicy::ON_ENGAGE &&
          layer.get_vertical_scroll_bar().isVisible()) {
        return layer.get_vertical_scroll_bar().width();
      }
      return 0;
    }();
    auto height = [&] {
      if(horizontal_policy != ScrollBox::DisplayPolicy::ON_ENGAGE &&
          layer.get_horizontal_scroll_bar().isVisible()) {
        return layer.get_horizontal_scroll_bar().height();
      }
      return 0;
    }();
    return {width, height};
  }

  QSize toSize(QMargins margins) {
    return QSize(
      margins.left() + margins.right(), margins.top() + margins.bottom());
  }

  enum class ScrollBarVisibility {
    HIDDEN,
    CONDITIONAL,
    VISIBLE
  };

  ScrollBarVisibility to_visibility(ScrollBox::DisplayPolicy display_policy,
      QSizePolicy::Policy size_policy, int viewport, int size_hint,
      int minimum_size_hint, int minimum_size, int maximum_size, int padding,
      int scrollbar_size) {
    if(display_policy == ScrollBox::DisplayPolicy::NEVER) {
      return ScrollBarVisibility::HIDDEN;
    } else if(display_policy == ScrollBox::DisplayPolicy::ALWAYS) {
      return ScrollBarVisibility::VISIBLE;
    }
    auto size = [&] {
      if(size_policy == QSizePolicy::Fixed ||
          size_policy == QSizePolicy::Minimum ||
          size_policy == QSizePolicy::MinimumExpanding) {
        return std::clamp(
          size_hint, std::max(minimum_size_hint, minimum_size), maximum_size);
      }
      return std::max(minimum_size_hint, minimum_size);
    }();
    if(size <= viewport - padding - scrollbar_size) {
      return ScrollBarVisibility::HIDDEN;
    } else if(size <= viewport - padding) {
      return ScrollBarVisibility::CONDITIONAL;
    }
    return ScrollBarVisibility::VISIBLE;
  }

  std::tuple<bool, bool> to_scroll_bar_visibility(
      ScrollBox::DisplayPolicy horizontal_policy,
      ScrollBox::DisplayPolicy vertical_policy, const QSizePolicy& size_policy,
      QSize viewport, QSize size_hint, QSize minimum_size_hint,
      QSize minimum_size, QSize maximum_size, QSize padding,
      QSize scroll_bar_size) {
    auto horizontal_visibility = to_visibility(
      horizontal_policy, size_policy.horizontalPolicy(), viewport.width(),
      size_hint.width(), minimum_size_hint.width(), minimum_size.width(),
      maximum_size.width(), padding.width(), scroll_bar_size.width());
    auto vertical_visibility = to_visibility(
      vertical_policy, size_policy.verticalPolicy(), viewport.height(),
      size_hint.height(), minimum_size_hint.height(), minimum_size.height(),
      maximum_size.height(), padding.height(), scroll_bar_size.height());
    if(horizontal_visibility == ScrollBarVisibility::CONDITIONAL &&
        vertical_visibility == ScrollBarVisibility::CONDITIONAL ||
        horizontal_visibility == ScrollBarVisibility::HIDDEN &&
        vertical_visibility == ScrollBarVisibility::HIDDEN ||
        horizontal_visibility == ScrollBarVisibility::HIDDEN &&
        vertical_visibility == ScrollBarVisibility::CONDITIONAL ||
        horizontal_visibility == ScrollBarVisibility::CONDITIONAL &&
        vertical_visibility == ScrollBarVisibility::HIDDEN) {
      return {false, false};
    } else if(horizontal_visibility == ScrollBarVisibility::VISIBLE &&
        vertical_visibility == ScrollBarVisibility::VISIBLE ||
        horizontal_visibility == ScrollBarVisibility::CONDITIONAL &&
        vertical_visibility == ScrollBarVisibility::VISIBLE ||
        horizontal_visibility == ScrollBarVisibility::VISIBLE &&
        vertical_visibility == ScrollBarVisibility::CONDITIONAL) {
      return {true, true};
    } else if(horizontal_visibility == ScrollBarVisibility::HIDDEN) {
      return {false, true};
    } else if(vertical_visibility == ScrollBarVisibility::HIDDEN) {
      return {true, false};
    }
    return {false, false};
  }

  int policy_to_size(QSizePolicy::Policy policy, int viewport, int size_hint,
      int minimum_size_hint, int minimum_size, int maximum_size, int padding) {
    if(policy == QSizePolicy::Fixed) {
      return std::clamp(size_hint, std::max(minimum_size_hint, minimum_size),
        maximum_size);
    } else if(policy == QSizePolicy::Minimum) {
      return std::clamp(viewport - padding, std::max(
        {size_hint, minimum_size_hint, minimum_size}), maximum_size);
    } else if(policy == QSizePolicy::Maximum) {
      return std::clamp(viewport - padding, std::max(
        minimum_size_hint, minimum_size), std::min(size_hint, maximum_size));
    } else if(
        policy == QSizePolicy::Preferred || policy == QSizePolicy::Ignored) {
      return std::clamp(viewport - padding, std::max(
        minimum_size_hint, minimum_size), maximum_size);
    } else if(policy == QSizePolicy::Expanding) {
      return std::clamp(viewport - padding, std::max(
        minimum_size_hint, minimum_size), maximum_size);
    } else if(policy == QSizePolicy::MinimumExpanding) {
      return std::clamp(viewport - padding, std::max(
        {size_hint, minimum_size_hint, minimum_size}), maximum_size);
    }
    return 0;
  }

  void update_scroll_bar_style(ScrollBar& scroll_bar, ScrollBox::DisplayPolicy policy) {
    if(policy == ScrollBox::DisplayPolicy::ON_ENGAGE) {
      update_style(scroll_bar, [] (auto& style) {
        style.get(Any() > ScrollThumb()).
          set(BackgroundColor(QColor("#CCC8C8C8")));
        style.get(Any() > (ScrollThumb() && (Hover() || Drag()))).
          set(BackgroundColor(QColor("#CCA0A0A0")));
        style.get(Any() > ScrollTrack()).
          set(BackgroundColor(QColor("#CCFFFFFF")));
      });
    } else {
      update_style(scroll_bar, [] (auto& style) {
        style.get(Any() > ScrollThumb()).
          set(BackgroundColor(QColor(0xC8C8C8)));
        style.get(Any() > (ScrollThumb() && (Hover() || Drag()))).
          set(BackgroundColor(QColor(0xA0A0A0)));
        style.get(Any() > ScrollTrack()).
          set(BackgroundColor(QColor(0xFFFFFF)));
      });
    }
  }

  struct Viewport : QWidget {
    QWidget* m_body;

    Viewport(QWidget& body)
        : m_body(&body) {
      setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      setObjectName(QString("0x%1").arg(reinterpret_cast<std::intptr_t>(this)));
      m_body->setParent(this);
    }

    QSize sizeHint() const override {
      return m_body->sizeHint();
    }

    bool event(QEvent* event) override {
      if(event->type() == QEvent::LayoutRequest) {
        updateGeometry();
      }
      return QWidget::event(event);
    }
  };
}

struct ScrollBox::ScrollBarAnimation : public QPropertyAnimation {
  ScrollBar* m_scroll_bar;

  ScrollBarAnimation(ScrollBar& scroll_bar)
    : QPropertyAnimation(&scroll_bar, ""),
      m_scroll_bar(&scroll_bar) {}

  void updateCurrentValue(const QVariant& value) {
    if(m_scroll_bar->get_orientation() == Qt::Vertical) {
      m_scroll_bar->setFixedWidth(value.toInt());
    } else {
      m_scroll_bar->setFixedHeight(value.toInt());
    }
  }
};

ScrollBox::ScrollBox(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body),
      m_horizontal_display_policy(DisplayPolicy::ON_OVERFLOW),
      m_vertical_display_policy(DisplayPolicy::ON_OVERFLOW),
      m_hover_observer(*this),
      m_border_styles([=] { commit_border_styles(); }),
      m_padding_styles([=] { commit_padding_styles(); }) {
  setFocusPolicy(Qt::StrongFocus);
  setObjectName(QString("0x%1").arg(reinterpret_cast<std::intptr_t>(this)));
  match(*m_body, Body());
  m_viewport = new Viewport(*m_body);
  m_body->installEventFilter(this);
  auto layers = new LayeredWidget();
  layers->add(m_viewport);
  m_scrollable_layer = new ScrollableLayer();
  m_scrollable_layer->setSizePolicy(
    QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto& vertical_scroll_bar = m_scrollable_layer->get_vertical_scroll_bar();
  auto& horizontal_scroll_bar = m_scrollable_layer->get_horizontal_scroll_bar();
  vertical_scroll_bar.setVisible(false);
  horizontal_scroll_bar.setVisible(false);
  vertical_scroll_bar.connect_position_signal(
    [=] (auto position) { on_vertical_scroll(position); });
  horizontal_scroll_bar.connect_position_signal(
    [=] (auto position) { on_horizontal_scroll(position); });
  horizontal_scroll_bar.installEventFilter(this);
  vertical_scroll_bar.installEventFilter(this);
  m_horizontal_bar_hover_observer.emplace(horizontal_scroll_bar);
  m_vertical_bar_hover_observer.emplace(vertical_scroll_bar);
  m_horizontal_bar_animation =
    std::make_unique<ScrollBarAnimation>(horizontal_scroll_bar);
  m_vertical_bar_animation =
    std::make_unique<ScrollBarAnimation>(vertical_scroll_bar);
  layers->add(m_scrollable_layer);
  enclose(*this, *layers);
  m_style_connection = connect_style_signal(*this, [=] { on_style(); });
  m_hover_observer.connect_state_signal(
    std::bind_front(&ScrollBox::on_hover, this));
  m_horizontal_bar_hover_observer->connect_state_signal(
    std::bind_front(&ScrollBox::on_horizontal_bar_hover, this));
  m_vertical_bar_hover_observer->connect_state_signal(
    std::bind_front(&ScrollBox::on_vertical_bar_hover, this));
}

const QWidget& ScrollBox::get_body() const {
  return *m_body;
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
  update_scroll_bar_style(m_scrollable_layer->get_horizontal_scroll_bar(),
    m_horizontal_display_policy);
  if(m_horizontal_display_policy == DisplayPolicy::ON_ENGAGE) {
    if(m_hover_observer.get_state() == HoverObserver::State::NONE) {
      m_scrollable_layer->get_horizontal_scroll_bar().setFixedHeight(0);
    } else {
      ease_in_horizontal_scroll_bar(NARROW_SCROLL_BAR_SIZE().height());
    }
  } else {
    m_scrollable_layer->get_horizontal_scroll_bar().setMinimumSize(0, 0);
    m_scrollable_layer->get_horizontal_scroll_bar().setMaximumSize(
      QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
  }
  update_layout();
}

ScrollBox::DisplayPolicy ScrollBox::get_vertical_display_policy() const {
  return m_vertical_display_policy;
}

void ScrollBox::set_vertical(DisplayPolicy policy) {
  if(policy == m_vertical_display_policy) {
    return;
  }
  m_vertical_display_policy = policy;
  update_scroll_bar_style(m_scrollable_layer->get_vertical_scroll_bar(),
    m_vertical_display_policy);
  if(m_vertical_display_policy == DisplayPolicy::ON_ENGAGE) {
    if(m_hover_observer.get_state() == HoverObserver::State::NONE) {
      m_scrollable_layer->get_vertical_scroll_bar().setFixedWidth(0);
    } else {
      ease_in_vertical_scroll_bar(NARROW_SCROLL_BAR_SIZE().width());
    }
  } else {
    m_scrollable_layer->get_vertical_scroll_bar().setMinimumSize(0, 0);
    m_scrollable_layer->get_vertical_scroll_bar().setMaximumSize(
      QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
  }
  update_layout();
}

void ScrollBox::set(DisplayPolicy policy) {
  set(policy, policy);
}

void ScrollBox::set(
    DisplayPolicy horizontal_policy, DisplayPolicy vertical_policy) {
  set_horizontal(horizontal_policy);
  set_vertical(vertical_policy);
}

ScrollBar& ScrollBox::get_vertical_scroll_bar() {
  return m_scrollable_layer->get_vertical_scroll_bar();
}

ScrollBar& ScrollBox::get_horizontal_scroll_bar() {
  return m_scrollable_layer->get_horizontal_scroll_bar();
}

void ScrollBox::scroll_to(const QWidget& widget) {
  auto widget_position =
    m_body->mapFromGlobal(widget.parentWidget()->mapToGlobal(widget.pos()));
  auto scroll_bar_size = get_scroll_bar_size(*m_scrollable_layer,
    m_horizontal_display_policy, m_vertical_display_policy);
  auto visible_width = m_viewport->width() - scroll_bar_size.width();
  auto x_scroll_position =
    m_scrollable_layer->get_horizontal_scroll_bar().get_position();
  if(widget.width() > visible_width ||
      x_scroll_position > widget_position.x()) {
    m_scrollable_layer->get_horizontal_scroll_bar().set_position(
      widget_position.x());
  } else {
    auto right_point = widget_position.x() + widget.width();
    if(x_scroll_position + visible_width < right_point) {
      m_scrollable_layer->get_horizontal_scroll_bar().set_position(
        right_point - visible_width);
    }
  }
  auto visible_height = m_viewport->height() - scroll_bar_size.height();
  auto y_scroll_position =
    m_scrollable_layer->get_vertical_scroll_bar().get_position();
  if(widget.height() > visible_height ||
      y_scroll_position > widget_position.y()) {
    m_scrollable_layer->get_vertical_scroll_bar().set_position(
      widget_position.y());
  } else {
    auto bottom_point = widget_position.y() + widget.height();
    if(y_scroll_position + visible_height < bottom_point) {
      m_scrollable_layer->get_vertical_scroll_bar().set_position(
        bottom_point - visible_height);
    }
  }
}

QSize ScrollBox::sizeHint() const {
  auto size = m_body->sizeHint() + toSize(m_borders) + toSize(m_padding);
  if(m_vertical_display_policy != DisplayPolicy::ON_ENGAGE) {
    if(m_vertical_display_policy == DisplayPolicy::ALWAYS ||
        !m_scrollable_layer->get_vertical_scroll_bar().isHidden()) {
      size.rwidth() +=
        m_scrollable_layer->get_vertical_scroll_bar().sizeHint().width();
    }
  }
  if(m_horizontal_display_policy != DisplayPolicy::ON_ENGAGE) {
    if(m_horizontal_display_policy == DisplayPolicy::ALWAYS ||
        !m_scrollable_layer->get_horizontal_scroll_bar().isHidden()) {
      size.rheight() +=
        m_scrollable_layer->get_horizontal_scroll_bar().sizeHint().height();
    }
  }
  return size;
}

bool ScrollBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body) {
    if(event->type() == QEvent::Resize) {
      update_layout();
    }
  } else if(event->type() == QEvent::Show || event->type() == QEvent::Hide) {
    update_ranges();
  }
  return QWidget::eventFilter(watched, event);
}

bool ScrollBox::event(QEvent* event) {
  if(event->type() == QEvent::LayoutRequest) {
    update_layout();
  }
  return QWidget::event(event);
}

void ScrollBox::keyPressEvent(QKeyEvent* event) {
  m_scrollable_layer->keyPressEvent(event);
}

void ScrollBox::resizeEvent(QResizeEvent* event) {
  update_layout();
}

void ScrollBox::wheelEvent(QWheelEvent* event) {
  m_scrollable_layer->wheelEvent(event);
}

void ScrollBox::showEvent(QShowEvent* event) {
  update_ranges();
}

void ScrollBox::ease(ScrollBarAnimation& animation, int size,
    const time_duration& duration, QEasingCurve::Type type) {
  if(!animation.m_scroll_bar->isVisible()) {
    return;
  }
  animation.setEasingCurve(type);
  animation.setDuration(duration.total_milliseconds());
  if(animation.m_scroll_bar->get_orientation() == Qt::Vertical) {
    animation.setStartValue(animation.m_scroll_bar->width());
  } else {
    animation.setStartValue(animation.m_scroll_bar->height());
  }
  animation.setEndValue(size);
  animation.start();
}

void ScrollBox::ease_in_horizontal_scroll_bar(int size) {
  ease(*m_horizontal_bar_animation, size, EASE_DURATION, QEasingCurve::InCubic);
}

void ScrollBox::ease_in_vertical_scroll_bar(int size) {
  ease(*m_vertical_bar_animation, size, EASE_DURATION, QEasingCurve::InCubic);
}

void ScrollBox::ease_out_horizontal_scroll_bar(int size) {
  ease(*m_horizontal_bar_animation, size, EASE_DURATION,
    QEasingCurve::OutCubic);
}

void ScrollBox::ease_out_vertical_scroll_bar(int size) {
  ease(*m_vertical_bar_animation, size, EASE_DURATION, QEasingCurve::OutCubic);
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
  }
}

void ScrollBox::commit_padding_styles() {
  auto stylesheet =
    QString("#0x%1 {").arg(reinterpret_cast<std::intptr_t>(m_viewport));
  m_padding_styles.write(stylesheet);
  if(stylesheet != m_viewport->styleSheet()) {
    m_viewport->setStyleSheet(stylesheet);
  }
  on_horizontal_scroll(
    m_scrollable_layer->get_horizontal_scroll_bar().get_position());
  on_vertical_scroll(
    m_scrollable_layer->get_vertical_scroll_bar().get_position());
  update_layout();
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
  m_body->move(m_body->pos().x(), -position + m_padding.top());
}

void ScrollBox::on_horizontal_scroll(int position) {
  m_body->move(-position + m_padding.left(), m_body->pos().y());
}

void ScrollBox::update_layout() {
  auto padding = toSize(m_padding);
  auto scroll_bar_width = [&] {
    if(m_vertical_display_policy == DisplayPolicy::ON_ENGAGE) {
      return 0;
    }
    return m_scrollable_layer->get_vertical_scroll_bar().sizeHint().width();
  }();
  auto scroll_bar_height = [&] {
    if(m_horizontal_display_policy == DisplayPolicy::ON_ENGAGE) {
      return 0;
    }
    return m_scrollable_layer->get_horizontal_scroll_bar().sizeHint().height();
  }();
  auto scroll_bar_size = QSize(scroll_bar_width, scroll_bar_height);
  auto viewport_size = [&] {
    auto size = m_viewport->maximumSize();
    if(size.width() == QWIDGETSIZE_MAX) {
      size.setWidth(m_viewport->size().width());
    }
    if(size.height() == QWIDGETSIZE_MAX) {
      size.setHeight(m_viewport->size().height());
    }
    return size;
  }();
  auto [is_horizontal_scroll_bar_shown, is_vertical_scroll_bar_shown] =
    to_scroll_bar_visibility(m_horizontal_display_policy,
      m_vertical_display_policy, m_body->sizePolicy(), viewport_size,
      m_body->sizeHint(), m_body->minimumSizeHint(), m_body->minimumSize(),
      m_body->maximumSize(), padding, scroll_bar_size);
  if(!is_horizontal_scroll_bar_shown) {
    scroll_bar_size.rheight() = 0;
  }
  if(!is_vertical_scroll_bar_shown) {
    scroll_bar_size.rwidth() = 0;
  }
  auto body_size = [&] {
    auto body_width = policy_to_size(m_body->sizePolicy().horizontalPolicy(),
      m_viewport->width() - scroll_bar_size.width(), m_body->sizeHint().width(),
      m_body->minimumSizeHint().width(), m_body->minimumWidth(),
      m_body->maximumWidth(), padding.width());
    auto body_height = policy_to_size(m_body->sizePolicy().verticalPolicy(),
      m_viewport->height() - scroll_bar_size.height(),
      m_body->sizeHint().height(), m_body->minimumSizeHint().height(),
      m_body->minimumHeight(), m_body->maximumHeight(), padding.height());
    return QSize(body_width, body_height);
  }();
  if(body_size != m_body->size()) {
    m_body->resize(body_size);
    return;
  }
  auto contents_size = m_body->size() + toSize(m_borders) + padding;
  const auto EXPAND = QSizePolicy::GrowFlag | QSizePolicy::IgnoreFlag;
  if(!(m_body->sizePolicy().horizontalPolicy() & EXPAND)) {
    setMaximumWidth(std::min(
      contents_size.width() + scroll_bar_size.width(), QWIDGETSIZE_MAX));
  } else {
    setMaximumWidth(QWIDGETSIZE_MAX);
  }
  if(!(m_body->sizePolicy().verticalPolicy() & EXPAND)) {
    setMaximumHeight(std::min(
      contents_size.height() + scroll_bar_size.height(), QWIDGETSIZE_MAX));
  } else {
    setMaximumHeight(QWIDGETSIZE_MAX);
  }
  auto is_update_geometry_needed = false;
  if(m_scrollable_layer->get_horizontal_scroll_bar().isVisible() !=
      is_horizontal_scroll_bar_shown) {
    m_scrollable_layer->get_horizontal_scroll_bar().setVisible(
      is_horizontal_scroll_bar_shown);
    is_update_geometry_needed = true;
  }
  if(m_scrollable_layer->get_vertical_scroll_bar().isVisible() !=
      is_vertical_scroll_bar_shown) {
    m_scrollable_layer->get_vertical_scroll_bar().setVisible(
      is_vertical_scroll_bar_shown);
    is_update_geometry_needed = true;
  }
  if(is_update_geometry_needed) {
    updateGeometry();
  }
  update_ranges();
}

void ScrollBox::update_ranges() {
  auto scroll_bar_size = get_scroll_bar_size(*m_scrollable_layer,
    m_horizontal_display_policy, m_vertical_display_policy);
  auto range = m_body->size() + toSize(m_padding) + scroll_bar_size -
    m_viewport->size();
  m_scrollable_layer->get_vertical_scroll_bar().set_range(
    0, std::max(range.height(), 0));
  m_scrollable_layer->get_vertical_scroll_bar().set_page_size(
    m_viewport->height());
  m_scrollable_layer->get_horizontal_scroll_bar().set_range(
    0, std::max(range.width(), 0));
  m_scrollable_layer->get_horizontal_scroll_bar().set_page_size(
    m_viewport->width());
}

void ScrollBox::on_hover(HoverObserver::State state) {
  if(state == HoverObserver::State::NONE) {
    if(m_vertical_display_policy == DisplayPolicy::ON_ENGAGE) {
      ease_out_vertical_scroll_bar(0);
    }
    if(m_horizontal_display_policy == DisplayPolicy::ON_ENGAGE) {
      ease_out_horizontal_scroll_bar(0);
    }
  } else {
    if(m_vertical_display_policy == DisplayPolicy::ON_ENGAGE) {
      ease_in_vertical_scroll_bar(NARROW_SCROLL_BAR_SIZE().width());
    }
    if(m_horizontal_display_policy == DisplayPolicy::ON_ENGAGE) {
      ease_in_horizontal_scroll_bar(NARROW_SCROLL_BAR_SIZE().height());
    }
  }
}

void ScrollBox::on_horizontal_bar_hover(HoverObserver::State state) {
  if(m_horizontal_display_policy != DisplayPolicy::ON_ENGAGE) {
    return;
  }
  if(state != HoverObserver::State::NONE) {
    ease_in_horizontal_scroll_bar(WIDE_SCROLL_BAR_SIZE().height());
  } else if(m_hover_observer.get_state() != HoverObserver::State::NONE) {
    ease_out_horizontal_scroll_bar(NARROW_SCROLL_BAR_SIZE().height());
  }
}

void ScrollBox::on_vertical_bar_hover(HoverObserver::State state) {
  if(m_vertical_display_policy != DisplayPolicy::ON_ENGAGE) {
    return;
  }
  if(state != HoverObserver::State::NONE) {
    ease_in_vertical_scroll_bar(WIDE_SCROLL_BAR_SIZE().width());
  } else if(m_hover_observer.get_state() != HoverObserver::State::NONE) {
    ease_out_vertical_scroll_bar(NARROW_SCROLL_BAR_SIZE().width());
  }
}
