#include "Spire/Ui/ScrollableListBox.hpp"
#include <QResizeEvent>
#include <QHBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/ValueModel.hpp"
#include "Spire/Ui/ListView.hpp"
#include "Spire/Ui/ScrollBar.hpp"

using namespace Spire;
using namespace Spire::Styles;

ScrollableListBox::ScrollableListBox(ListView* list_view, QWidget* parent)
    : ScrollBox(make_body(), parent)
    , m_list_view(list_view) {
  m_list_view->set_edge_navigation(ListView::EdgeNavigation::CONTAIN);
  auto layout = [=] () -> QBoxLayout* {
    if(is_horizontal_layout()) {
      return new QHBoxLayout(m_body);
    } else {
      return new QVBoxLayout(m_body);
    }
  }();
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(list_view);
  if(is_horizontal_layout()) {
    layout->addSpacing(get_vertical_scroll_bar().sizeHint().width());
    set_horizontal(ScrollBox::DisplayPolicy::NEVER);
    set_vertical(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  } else {
    layout->addSpacing(get_horizontal_scroll_bar().sizeHint().height());
    set_horizontal(ScrollBox::DisplayPolicy::ON_OVERFLOW);
    set_vertical(ScrollBox::DisplayPolicy::NEVER);
  }
  auto style = get_style(*this);
  style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
    set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  set_style(*this, std::move(style));
  m_list_view->get_current_model()->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
  m_body->installEventFilter(this);
  m_list_view->installEventFilter(this);
}

QSize ScrollableListBox::sizeHint() const {
  return m_body_size + get_border_size();
}

bool ScrollableListBox::eventFilter(QObject* watched, QEvent* event) {
  if(watched == m_body && event->type() == QEvent::Resize) {
    if(!m_body_size.isValid()) {
      m_body_size = m_body->size();
    }
  }
  return ScrollBox::eventFilter(watched, event);
}

void ScrollableListBox::resizeEvent(QResizeEvent* event) {
  if(m_body_size.isValid()) {
    if(m_list_view->get_direction() == Qt::Vertical) {
      if(m_list_view->get_overflow() == ListView::Overflow::NONE) {
        m_body->resize({event->size().width() - get_border_size().width(),
          m_body->height()});
      } else {
        m_list_view->setFixedHeight(event->size().height() - get_bar_height() -
          get_border_size().height());
        m_body->resize({m_list_view->get_layout_size().width(),
          event->size().height() - get_border_size().height()});
      }
    } else {
      if(m_list_view->get_overflow() == ListView::Overflow::NONE) {
        m_body->resize({m_body->width(),
          event->size().height() - get_border_size().height()});
      } else {
        m_list_view->setFixedWidth(event->size().width() - get_bar_width() -
          get_border_size().width());
        m_body->resize({event->size().width() - get_border_size().width(),
          m_list_view->get_layout_size().height()});
      }
    }
  }
  ScrollBox::resizeEvent(event);
}

void ScrollableListBox::update_ranges() {
  auto border_size = get_border_size();
  auto viewport_size = m_list_view->size();
  if(is_horizontal_layout()) {
    if(viewport_size.height() <= height() - get_bar_height() -
        border_size.height()) {
      get_vertical_scroll_bar().hide();
    } else {
      get_vertical_scroll_bar().show();
    }
  } else {
    if(viewport_size.width() <= width() - get_bar_width() -
        border_size.width()) {
      get_horizontal_scroll_bar().hide();
    } else {
      get_horizontal_scroll_bar().show();
    }
  }
 auto new_size = size() - border_size -
    QSize{get_bar_width(), get_bar_height()};
  auto vertical_range = std::max(m_list_view->height() - new_size.height(), 0);
  auto horizontal_range = std::max(m_list_view->width() - new_size.width(), 0);
  get_vertical_scroll_bar().set_range(0, vertical_range);
  get_vertical_scroll_bar().set_page_size(new_size.height());
  get_horizontal_scroll_bar().set_range(0, horizontal_range);
  get_horizontal_scroll_bar().set_page_size(new_size.width());
}

void ScrollableListBox::on_current(const boost::optional<std::any>& current) {
  if(!current) {
    return;
  }
  auto bar_height = get_bar_height();
  auto bar_width = get_bar_width();
  auto item = m_list_view->get_item(*current);
  auto item_pos = item->pos();
  auto item_height = item->height();
  auto item_width = item->width();
  auto viewport_x = get_horizontal_scroll_bar().get_position();
  auto viewport_y = get_vertical_scroll_bar().get_position();
  auto viewport_width =
    get_horizontal_scroll_bar().get_page_size();
  auto viewport_height =
    get_vertical_scroll_bar().get_page_size();
  if(item_height > viewport_height || viewport_y > item_pos.y()) {
    get_vertical_scroll_bar().set_position(item_pos.y());
  } else if(viewport_y + viewport_height - bar_height <
      item_pos.y() + item_height) {
    get_vertical_scroll_bar().set_position(
      item_pos.y() + item_height - viewport_height + bar_height);
  }
  if(item_width > viewport_width || viewport_x > item_pos.x()) {
    get_horizontal_scroll_bar().set_position(item_pos.x());
  } else if(viewport_x + viewport_width - bar_width <
      item_pos.x() + item_width) {
    get_horizontal_scroll_bar().set_position(
      item_pos.x() + item_width - viewport_width + bar_width);
  }
}

QWidget* ScrollableListBox::make_body() {
  auto body = new QWidget();
  body->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_body = body;
  return body;
}

bool ScrollableListBox::is_horizontal_layout() {
  return (m_list_view->get_direction() == Qt::Vertical &&
    m_list_view->get_overflow() == ListView::Overflow::NONE) ||
    (m_list_view->get_direction() == Qt::Horizontal &&
      m_list_view->get_overflow() == ListView::Overflow::WRAP);
}

int ScrollableListBox::get_bar_width() {
  if(get_vertical_scroll_bar().isVisible()) {
    return get_vertical_scroll_bar().width();
  }
  return 0;
}

int ScrollableListBox::get_bar_height() {
  if(get_horizontal_scroll_bar().isVisible()) {
    return get_horizontal_scroll_bar().height();
  }
  return 0;
}
