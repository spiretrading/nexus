#include "Spire/Ui/DropDownMenuList.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto BORDER_PADDING = 2;
}

DropDownMenuList::DropDownMenuList(std::vector<DropDownMenuItem*> items,
    bool is_click_activated, QWidget* parent)
    : DropDownWindow(is_click_activated, parent),
      m_max_displayed_items(5) {
  m_scroll_area = new ScrollArea(this);
  m_scroll_area->setFocusProxy(parent);
  m_scroll_area->setWidgetResizable(true);
  auto main_widget = new QWidget(this);
  m_layout = new QVBoxLayout(main_widget);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  m_scroll_area->setWidget(main_widget);
  initialize_widget(m_scroll_area);
  set_items(items);
  parent->installEventFilter(this);
}

bool DropDownMenuList::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      switch(e->key()) {
        case Qt::Key_Down:
          if(isVisible()) {
            activate_next();
            return true;
          }
          break;
        case Qt::Key_Up:
          if(isVisible()) {
            activate_previous();
            return true;
          }
          break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
          if(isVisible() && m_highlight_index) {
            on_item_selected(get_item(*m_highlight_index)->get_value(),
              *m_highlight_index);
            return true;
          }
          break;
      }
    }
  } else if(isAncestorOf(static_cast<QWidget*>(watched))) {
    if(event->type() == QEvent::HoverEnter) {
      m_highlighted_signal(
        static_cast<DropDownMenuItem*>(watched)->get_value());
    }
  }
  return DropDownWindow::eventFilter(watched, event);
}

void DropDownMenuList::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Down:
      if(isVisible()) {
        activate_next();
      }
      break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(m_highlight_index) {
        on_item_selected(get_item(*m_highlight_index)->get_value(),
          *m_highlight_index);
      }
      break;
    case Qt::Key_Escape:
      hide();
      break;
    case Qt::Key_Up:
      if(isVisible()) {
        activate_previous();
      }
      break;
  }
  DropDownWindow::keyPressEvent(event);
}

connection DropDownMenuList::connect_activated_signal(
    const ActivatedSignal::slot_type& slot) const {
  return m_activated_signal.connect(slot);
}

connection DropDownMenuList::connect_highlighted_signal(
    const HighlightedSignal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

connection DropDownMenuList::connect_index_selected_signal(
    const IndexSelectedSignal::slot_type& slot) const {
  return m_index_selected_signal.connect(slot);
}

connection DropDownMenuList::connect_value_selected_signal(
    const ValueSelectedSignal::slot_type& slot) const {
  return m_value_selected_signal.connect(slot);
}

const QVariant& DropDownMenuList::get_value(int index) {
  if(m_layout->count() - 1 < index) {
    static auto SENTINEL = QVariant();
    return SENTINEL;
  }
  return get_item(index)->get_value();
}

void DropDownMenuList::insert_item(DropDownMenuItem* item) {
  m_item_selected_connections.AddConnection(item->connect_selected_signal(
    [=] (const auto& value) {
      on_item_selected(value, item);
    }));
  m_layout->insertWidget(m_layout->count(), item);
  update_height();
}

int DropDownMenuList::item_count() const {
  return m_layout->count();
}

void DropDownMenuList::remove_item(int index) {
  if(index > m_layout->count() - 1) {
    return;
  }
  auto layout_item = m_layout->takeAt(index);
  m_item_selected_connections.Disconnect(layout_item->widget());
  delete layout_item->widget();
  delete layout_item;
  if(index == m_highlight_index) {
    m_highlight_index = none;
  }
  update_height();
}

bool DropDownMenuList::set_highlight(const QString& text) {
  for(auto i = 0; i < item_count(); ++i) {
    if(m_item_delegate.displayText(get_value(i)).startsWith(text,
        Qt::CaseInsensitive)) {
      set_highlight(i);
      return true;
    }
  }
  return false;
}

void DropDownMenuList::set_items(const std::vector<DropDownMenuItem*>& items) {
  while(auto item = m_layout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  m_item_selected_connections.DisconnectAll();
  for(auto& item : items) {
    m_layout->addWidget(item);
    item->installEventFilter(this);
    m_item_selected_connections.AddConnection(item->connect_selected_signal(
      [=] (const auto& value) {
        on_item_selected(value, item);
      }));
  }
  if(m_layout->count() > 0) {
    update_height();
    static_cast<DropDownMenuItem*>(
      m_layout->itemAt(0)->widget())->set_highlight();
    m_highlight_index = 0;
  } else {
    m_highlight_index = none;
    hide();
  }
}

DropDownMenuItem* DropDownMenuList::get_item(int index) {
  return static_cast<DropDownMenuItem*>(m_layout->itemAt(index)->widget());
}

void DropDownMenuList::activate_next() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(0);
    return;
  }
  auto next_index = [&] {
    auto index = *m_highlight_index + 1;
    if(index > m_layout->count() - 1) {
      return 0;
    }
    return index;
  }();
  set_highlight(next_index);
}

void DropDownMenuList::activate_previous() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(m_layout->count() - 1);
    return;
  }
  auto previous_index = [&] {
    auto index = *m_highlight_index - 1;
    if(index < 0) {
      return m_layout->count() - 1;
    }
    return index;
  }();
  set_highlight(previous_index);
}

void DropDownMenuList::clear_active_item() {
  if(m_highlight_index) {
    get_item(*m_highlight_index)->reset_highlight();
    m_highlight_index = none;
  }
}

void DropDownMenuList::set_highlight(int index) {
  if(m_highlight_index) {
    get_item(*m_highlight_index)->reset_highlight();
  }
  m_highlight_index = index;
  auto highlighted_widget = get_item(*m_highlight_index);
  highlighted_widget->set_highlight();
  scroll_to_highlight();
  m_activated_signal(highlighted_widget->get_value());
}

void DropDownMenuList::scroll_to_highlight() {
  if(m_highlight_index != m_layout->count() - 1) {
    m_scroll_area->ensureWidgetVisible(get_item(*m_highlight_index), 0,
      0);
  } else {
    m_scroll_area->verticalScrollBar()->setValue(
      m_scroll_area->verticalScrollBar()->maximum());
  }
}

void DropDownMenuList::update_height() {
  if(m_layout->count() == 0) {
    return;
  }
  setFixedHeight(std::min(m_max_displayed_items, m_layout->count()) *
    m_layout->itemAt(0)->widget()->height() + BORDER_PADDING);
}

void DropDownMenuList::on_item_selected(QVariant value, int index) {
  m_index_selected_signal(index);
  m_value_selected_signal(std::move(value));
  hide();
}

void DropDownMenuList::on_item_selected(QVariant value,
    DropDownMenuItem* item) {
  on_item_selected(value, m_layout->indexOf(item));
}
