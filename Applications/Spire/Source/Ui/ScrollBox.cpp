#include "Spire/Ui/ScrollBox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"

using namespace Spire;

ScrollBox::ScrollBox(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body) {
  auto layers = new LayeredWidget(this);
  auto viewport = new QWidget();
  m_body->installEventFilter(this);
  m_body->setParent(viewport);
  layers->add(viewport);
  m_scrollable_layer = new ScrollableLayer();
  m_scrollable_layer->get_vertical_scroll_bar().connect_position_signal(
    [=] (auto position) { on_vertical_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().connect_position_signal(
    [=] (auto position) { on_horizontal_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().installEventFilter(this);
  m_scrollable_layer->get_vertical_scroll_bar().installEventFilter(this);
  layers->add(m_scrollable_layer);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(layers);
  setFocusPolicy(Qt::StrongFocus);
  update_ranges();
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

void ScrollBox::set(DisplayPolicy horizontal_policy,
    DisplayPolicy vertical_policy) {
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
  //qDebug() << "bar_width: " << bar_width;
  auto viewport_size = m_body->size() + QSize(bar_width, bar_height);
  qDebug() << "vp_size: " << viewport_size;
  static auto vp = viewport_size;
  if(vp.height() < viewport_size.height()) {
    auto a = 0;
  }
  vp = viewport_size;
  setMaximumSize(viewport_size);
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
  //qDebug() << "vert_range: " << vertical_range;
  auto horizontal_range = std::max(m_body->width() - width() + bar_width, 0);
  m_scrollable_layer->get_vertical_scroll_bar().set_range(0, vertical_range);
  m_scrollable_layer->get_vertical_scroll_bar().set_page_size(height());
  m_scrollable_layer->get_horizontal_scroll_bar().set_range(
    0, horizontal_range);
  m_scrollable_layer->get_horizontal_scroll_bar().set_page_size(width());
}
