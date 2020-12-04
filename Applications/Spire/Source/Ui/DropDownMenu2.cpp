#include "Spire/Ui/DropDownMenu2.hpp"
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

DropDownMenu2::DropDownMenu2(std::vector<DropDownItem*> items,
    QWidget* parent)
    : DropDownWindow(true, parent),
      m_selected_index(0),
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

bool DropDownMenu2::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      switch(e->key()) {
        case Qt::Key_Down:
          if(isVisible()) {
            increment_current();
            return true;
          }
          break;
        case Qt::Key_Up:
          if(isVisible()) {
            decrement_current();
            return true;
          }
          break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
          if(isVisible() && m_current_index) {
            on_item_selected(get_item(*m_current_index)->get_value(),
              *m_current_index);
            return true;
          }
          break;
      }
    }
  }
  return DropDownWindow::eventFilter(watched, event);
}

void DropDownMenu2::hideEvent(QHideEvent* event) {
  m_closed_signal();
  DropDownWindow::hideEvent(event);
}

void DropDownMenu2::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Down:
      if(isVisible()) {
        increment_current();
      }
      break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
      if(m_current_index) {
        on_item_selected(get_item(*m_current_index)->get_value(),
          *m_current_index);
      }
      break;
    case Qt::Key_Escape:
      hide();
      break;
    case Qt::Key_Up:
      if(isVisible()) {
        decrement_current();
      }
      break;
  }
  DropDownWindow::keyPressEvent(event);
}

connection DropDownMenu2::connect_closed_signal(
    const ClosedSignal::slot_type& slot) const {
  return m_closed_signal.connect(slot);
}

connection DropDownMenu2::connect_current_signal(
    const CurrentSignal::slot_type& slot) const {
  return m_current_signal.connect(slot);
}

connection DropDownMenu2::connect_hovered_signal(
    const HoveredSignal::slot_type& slot) const {
  return m_hovered_signal.connect(slot);
}

connection DropDownMenu2::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void DropDownMenu2::increment_current() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_current_index) {
    set_current_index(0);
    return;
  }
  auto next_index = [&] {
    auto index = *m_current_index + 1;
    if(index > m_layout->count() - 1) {
      return 0;
    }
    return index;
  }();
  set_current_index(next_index);
}

const QVariant& DropDownMenu2::get_current_item() const {
  if(m_selected_index) {
    return get_value(*m_selected_index);
  }
  static auto SENTINEL = QVariant();
  return SENTINEL;
}

boost::optional<int> DropDownMenu2::get_selected_index() const {
  return m_selected_index;
}

int DropDownMenu2::item_count() const {
  return m_layout->count();
}

const QVariant& DropDownMenu2::get_value(int index) const {
  if(m_layout->count() - 1 < index) {
    static auto SENTINEL = QVariant();
    return SENTINEL;
  }
  return get_item(index)->get_value();
}

void DropDownMenu2::insert_item(DropDownItem* item) {
  m_item_hovered_connections.AddConnection(item->connect_highlighted_signal(
    [=] (const auto& value) { m_hovered_signal(item->get_value()); }));
  m_item_selected_connections.AddConnection(item->connect_selected_signal(
    [=] (const auto& value) { on_item_selected(item->get_value(), item); }));
  m_layout->insertWidget(m_layout->count(), item);
  update_height();
}

void DropDownMenu2::remove_item(int index) {
  if(index > m_layout->count() - 1) {
    return;
  }
  auto layout_item = m_layout->takeAt(index);
  m_item_selected_connections.Disconnect(layout_item->widget());
  delete layout_item->widget();
  delete layout_item;
  if(index == m_current_index) {
    m_current_index = none;
    m_current_signal({});
  }
  if(index == m_selected_index) {
    m_selected_index = none;
    m_selected_signal({});
  }
  update_height();
}

bool DropDownMenu2::set_current_item(const QString& text) {
  for(auto i = 0; i < item_count(); ++i) {
    if(m_item_delegate.displayText(get_value(i)).startsWith(text,
        Qt::CaseInsensitive)) {
      set_current_index(i);
      return true;
    }
  }
  return false;
}

void DropDownMenu2::set_items(const std::vector<DropDownItem*>& items) {
  while(auto item = m_layout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  m_item_hovered_connections.DisconnectAll();
  m_item_selected_connections.DisconnectAll();
  for(auto& item : items) {
    m_layout->addWidget(item);
    m_item_hovered_connections.AddConnection(item->connect_highlighted_signal(
      [=] (const auto& value) { m_hovered_signal(item->get_value()); }));
    m_item_selected_connections.AddConnection(item->connect_selected_signal(
      [=] (const auto& value) { on_item_selected(item->get_value(), item); }));
  }
  if(m_layout->count() > 0) {
    update_height();
    static_cast<DropDownItem*>(m_layout->itemAt(0)->widget())->set_highlight();
    m_selected_index = 0;
    m_current_index = 0;
    m_selected_signal(get_value(*m_selected_index));
    m_current_signal(get_value(*m_current_index));
  } else {
    m_selected_index = none;
    m_current_index = none;
    m_selected_signal({});
    m_current_signal({});
    hide();
  }
}

void DropDownMenu2::decrement_current() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_current_index) {
    set_current_index(m_layout->count() - 1);
    return;
  }
  auto previous_index = [&] {
    auto index = *m_current_index - 1;
    if(index < 0) {
      return m_layout->count() - 1;
    }
    return index;
  }();
  set_current_index(previous_index);
}

DropDownItem* DropDownMenu2::get_item(int index) const {
  return static_cast<DropDownItem*>(m_layout->itemAt(index)->widget());
}

void DropDownMenu2::set_current_index(int index) {
  if(m_current_index) {
    get_item(*m_current_index)->reset_highlight();
  }
  m_current_index = index;
  auto highlighted_widget = get_item(*m_current_index);
  highlighted_widget->set_highlight();
  scroll_to_current_index();
  m_current_signal(highlighted_widget->get_value());
}

void DropDownMenu2::scroll_to_current_index() {
  if(m_current_index != m_layout->count() - 1) {
    m_scroll_area->ensureWidgetVisible(get_item(*m_current_index), 0, 0);
  } else {
    m_scroll_area->verticalScrollBar()->setValue(
      m_scroll_area->verticalScrollBar()->maximum());
  }
}

void DropDownMenu2::update_height() {
  if(m_layout->count() == 0) {
    return;
  }
  setFixedHeight(std::min(m_max_displayed_items, m_layout->count()) *
    m_layout->itemAt(0)->widget()->height() + BORDER_PADDING);
}

void DropDownMenu2::on_item_selected(const QVariant& value, int index) {
  hide();
  m_current_index = index;
  m_selected_index = index;
  m_current_signal(value);
  m_selected_signal(value);
}

void DropDownMenu2::on_item_selected(const QVariant& value,
    DropDownItem* item) {
  on_item_selected(value, m_layout->indexOf(item));
}
