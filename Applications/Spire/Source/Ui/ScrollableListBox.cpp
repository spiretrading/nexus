#include "Spire/Ui/ScrollableListBox.hpp"
#include <QCoreApplication>
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
  m_list_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_list_view->set_edge_navigation(ListView::EdgeNavigation::CONTAIN);
  auto layout = [=] () -> QBoxLayout* {
    if(is_horizontal_layout()) {
      return new QHBoxLayout(m_body);
    }
    return new QVBoxLayout(m_body);
  }();
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  layout->addWidget(list_view);
  if(is_horizontal_layout()) {
    m_padding_size = get_vertical_scroll_bar().sizeHint().width();
  } else {
    m_padding_size = get_horizontal_scroll_bar().sizeHint().height();
  }
  layout->addSpacing(m_padding_size);
  m_scroll_bar_padding = layout->itemAt(1);
  if(is_horizontal_layout()) {
    set_horizontal(ScrollBox::DisplayPolicy::NEVER);
    set_vertical(ScrollBox::DisplayPolicy::ON_OVERFLOW);
  } else {
    set_horizontal(ScrollBox::DisplayPolicy::ON_OVERFLOW);
    set_vertical(ScrollBox::DisplayPolicy::NEVER);
  }
  auto style = get_style(*this);
  style.get(Any()).
    set(BackgroundColor(QColor::fromRgb(0xFF, 0xFF, 0xFF))).
    set(border(scale_width(1), QColor::fromRgb(0xC8, 0xC8, 0xC8)));
  set_style(*this, std::move(style));
  m_list_view_current_connection =
    m_list_view->get_current_model()->connect_current_signal(
    [=] (const auto& current) { on_current(current); });
}

QSize ScrollableListBox::sizeHint() const {
  auto border_size = get_border_size();
  if(is_horizontal_layout()) {
    return m_list_view->sizeHint() + QSize(m_padding_size, 0) + border_size;
  }
  return m_list_view->sizeHint() + QSize(0, m_padding_size) + border_size;
}

void ScrollableListBox::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Home:
    case Qt::Key_End:
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    case Qt::Key_Down:
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
      return;
  }
  ScrollBox::keyPressEvent(event);
}

void ScrollableListBox::resizeEvent(QResizeEvent* event) {
  auto border_size = get_border_size();
  auto update_list_view_width = [=] (int width) {
    if(!event->oldSize().isValid()) {
      QCoreApplication::postEvent(m_list_view, new QResizeEvent(
        QSize(width, m_list_view->height()), m_list_view->size()));
    }
  };
  auto update_list_view_height = [=] (int height) {
    if(!event->oldSize().isValid()) {
      QCoreApplication::postEvent(m_list_view, new QResizeEvent(
        QSize(m_list_view->width(), height), m_list_view->size()));
    }
  };
  auto set_maximum_height = [=] {
    auto height = m_list_view->sizeHint().height();
    if(height <= event->size().height() - border_size.height()) {
      setMaximumHeight(height + border_size.height());
    } else {
      setMaximumHeight(QWIDGETSIZE_MAX);
    }
  };
  auto set_maximum_width = [=] {
    auto width = m_list_view->sizeHint().width();
    if(width <= event->size().width() - border_size.width()) {
      setMaximumWidth(width + border_size.width());
    } else {
      setMaximumWidth(QWIDGETSIZE_MAX);
    }
  };
  auto get_horizontal_padding = [=] {
    if(m_list_view->sizeHint().height() <= event->size().height() -
        border_size.height()) {
      return 0;
    }
    return m_padding_size;
  };
  auto get_vertical_padding = [=] {
    if(m_list_view->sizeHint().width() <= event->size().width() -
        border_size.width()) {
      return 0;
    }
    return m_padding_size;
  };
  if(m_list_view->get_direction() == Qt::Vertical) {
    if(m_list_view->get_overflow() == ListView::Overflow::NONE) {
      auto width = event->size().width() - get_horizontal_padding() -
        border_size.width();
      m_list_view->setFixedWidth(width);
      update_list_view_width(width);
      set_maximum_height();
    } else {
      auto height = event->size().height() - border_size.height();
      m_list_view->setFixedHeight(height);
      update_list_view_height(height);
      auto padding = get_vertical_padding();
      if(padding > 0) {
        height -= padding;
        m_list_view->setFixedHeight(height);
        update_list_view_height(height);
      }
      set_maximum_width();
    }
  } else {
    if(m_list_view->get_overflow() == ListView::Overflow::NONE) {
      auto height = event->size().height() - get_vertical_padding() -
        border_size.height();
      m_list_view->setFixedHeight(height);
      update_list_view_height(height);
      set_maximum_width();
    } else {
      auto width = event->size().width() - border_size.width();
      m_list_view->setFixedWidth(width);
      update_list_view_width(width);
      auto padding = get_horizontal_padding();
      if(padding > 0) {
        width -= padding;
        m_list_view->setFixedWidth(width);
        update_list_view_width(width);
      }
      set_maximum_height();
    }
  }
  m_body->adjustSize();
  ScrollBox::resizeEvent(event);
}

void ScrollableListBox::update_ranges() {
  auto border_size = get_border_size();
  auto viewport_size = m_list_view->sizeHint();
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
  auto vertical_range = std::max(viewport_size.height() - new_size.height(), 0);
  auto horizontal_range = std::max(viewport_size.width() - new_size.width(), 0);
  if(vertical_range == 0 && horizontal_range == 0) {
    if(m_body->layout()->count() == 2) {
      m_body->layout()->removeItem(m_scroll_bar_padding);
    }
  } else if(m_body->layout()->count() == 1) {
    m_body->layout()->addItem(m_scroll_bar_padding);
  }
  get_vertical_scroll_bar().set_range(0, vertical_range);
  get_vertical_scroll_bar().set_page_size(new_size.height());
  get_horizontal_scroll_bar().set_range(0, horizontal_range);
  get_horizontal_scroll_bar().set_page_size(new_size.width());
}

void ScrollableListBox::on_current(const boost::optional<std::any>& current) {
  if(!current) {
    return;
  }
  auto item = m_list_view->get_item(*current);
  if(!item) {
    return;
  }
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
  } else if(viewport_y + viewport_height < item_pos.y() + item_height) {
    get_vertical_scroll_bar().set_position(
      item_pos.y() + item_height - viewport_height);
  }
  if(item_width > viewport_width || viewport_x > item_pos.x()) {
    get_horizontal_scroll_bar().set_position(item_pos.x());
  } else if(viewport_x + viewport_width < item_pos.x() + item_width) {
    get_horizontal_scroll_bar().set_position(
      item_pos.x() + item_width - viewport_width);
  }
}

QWidget* ScrollableListBox::make_body() {
  auto body = new QWidget();
  m_body = body;
  return body;
}

bool ScrollableListBox::is_horizontal_layout() const {
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
