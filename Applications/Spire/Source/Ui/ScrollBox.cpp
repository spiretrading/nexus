#include "Spire/Ui/ScrollBox.hpp"
#include <QEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/LayeredWidget.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBar.hpp"
#include "Spire/Ui/ScrollableLayer.hpp"

using namespace Spire;
using namespace Spire::Styles;

ScrollBox::ScrollBox(QWidget* body, QWidget* parent)
    : QWidget(parent),
      m_body(body) {
  setObjectName("scroll_box");
  auto layers = new LayeredWidget(this);
  layers->setObjectName("layered_widget");
  layers->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  auto viewport = new QWidget();
  viewport->setObjectName("scroll_box_viewport");
  m_body->installEventFilter(this);
  m_body->setParent(viewport);
  layers->add(viewport);
  m_scrollable_layer = new ScrollableLayer();
  m_scrollable_layer->setObjectName("scrollable_layer");
  m_scrollable_layer->get_vertical_scroll_bar().connect_position_signal(
    [=] (auto position) { on_vertical_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().connect_position_signal(
    [=] (auto position) { on_horizontal_scroll(position); });
  m_scrollable_layer->get_horizontal_scroll_bar().installEventFilter(this);
  m_scrollable_layer->get_vertical_scroll_bar().installEventFilter(this);
  layers->add(m_scrollable_layer);
  m_box = new Box(layers);
  proxy_style(*this, *m_box);
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins({});
  layout->addWidget(m_box);
  setFocusPolicy(Qt::StrongFocus);
  update_ranges();
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

void ScrollBox::set(DisplayPolicy horizontal_policy,
    DisplayPolicy vertical_policy) {
  set_horizontal(horizontal_policy);
  set_vertical(vertical_policy);
}

ScrollBar& ScrollBox::get_vertical_scroll_bar() {
  return m_scrollable_layer->get_vertical_scroll_bar();
}

ScrollBar& ScrollBox::get_horizontal_scroll_bar() {
  return m_scrollable_layer->get_horizontal_scroll_bar();
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
  auto border_size = QSize(0, 0);
  for(auto& property : get_evaluated_block(*m_box)) {
    property.visit(
      [&] (std::in_place_type_t<BorderTopSize>, int size) {
        border_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderRightSize>, int size) {
        border_size.rwidth() += size;
      },
      [&] (std::in_place_type_t<BorderBottomSize>, int size) {
        border_size.rheight() += size;
      },
      [&] (std::in_place_type_t<BorderLeftSize>, int size) {
        border_size.rwidth() += size;
      });
  }
  auto viewport_size = m_body->size() + QSize(bar_width, bar_height) +
    border_size;
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
  auto new_size = size() - border_size;
  auto vertical_range = std::max(m_body->height() - new_size.height() +
    bar_height, 0);
  auto horizontal_range = std::max(m_body->width() - new_size.width() +
    bar_width, 0);
  m_scrollable_layer->get_vertical_scroll_bar().set_range(0, vertical_range);
  m_scrollable_layer->get_vertical_scroll_bar().set_page_size(
    new_size.height());
  m_scrollable_layer->get_horizontal_scroll_bar().set_range(
    0, horizontal_range);
  m_scrollable_layer->get_horizontal_scroll_bar().set_page_size(
    new_size.width());
}

ScrollBox* Spire::make_scrollable_list_box(ListView* list_view,
    QWidget* parent) {
  list_view->set_edge_navigation(ListView::EdgeNavigation::CONTAIN);
  auto scroll_box = new ScrollBox(list_view, parent);
  scroll_box->set(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  auto style = get_style(*scroll_box);
  style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
    set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  set_style(*scroll_box, std::move(style));
  list_view->get_current_model()->connect_current_signal(
    [=] (const auto& current) {
      if(!current) {
        return;
      }
      auto item = list_view->get_item(*current);
      auto item_pos = item->pos();
      auto item_height = item->height();
      auto item_width = item->width();
      auto viewport_x = scroll_box->get_horizontal_scroll_bar().get_position();
      auto viewport_y = scroll_box->get_vertical_scroll_bar().get_position();
      auto viewport_width =
        scroll_box->get_horizontal_scroll_bar().get_page_size();
      auto viewport_height =
        scroll_box->get_vertical_scroll_bar().get_page_size();
      if(item_height > viewport_height || viewport_y > item_pos.y()) {
        scroll_box->get_vertical_scroll_bar().set_position(item_pos.y());
      } else if(viewport_y + viewport_height < item_pos.y() + item_height) {
        scroll_box->get_vertical_scroll_bar().set_position(
           item_pos.y() + item_height - viewport_height);
      }
      if(item_width > viewport_width || viewport_x > item_pos.x()) {
        scroll_box->get_horizontal_scroll_bar().set_position(item_pos.x());
      } else if(viewport_x + viewport_width < item_pos.x() + item_width) {
        scroll_box->get_horizontal_scroll_bar().set_position(
           item_pos.x() + item_width - viewport_width);
      }
    });
  return scroll_box;
}

std::unordered_set<Stylist*> BaseComponentFinder<ScrollBox, Body>::operator ()(
    ScrollBox& box, const Body& body) const {
  auto stylists = std::unordered_set<Stylist*>();
  stylists.insert(&find_stylist(box.get_body()));
  return stylists;
}
