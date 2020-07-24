#include "Spire/Ui/ListWidget.hpp"
#include <QFocusEvent>
#include <QKeyEvent>
#include <QScrollBar>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto BORDER_PADDING = 2;
}

ListWidget::ListWidget(std::vector<DropDownItem*> items, QWidget* parent)
    : ScrollArea(parent),
      m_max_displayed_items(5) {
  setWidgetResizable(true);
  auto main_widget = new QWidget(this);
  m_layout = new QVBoxLayout(main_widget);
  m_layout->setContentsMargins({});
  m_layout->setSpacing(0);
  setWidget(main_widget);
  set_items(items);
}

void ListWidget::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Down:
      if(isVisible()) {
        focus_next();
      }
      break;
    case Qt::Key_Enter:
      if(m_highlight_index) {
        on_item_selected(get_widget(*m_highlight_index)->get_value(),
          *m_highlight_index);
      }
      break;
    case Qt::Key_Up:
      if(isVisible()) {
        focus_previous();
      }
      break;
  }
  ScrollArea::keyPressEvent(event);
}

connection ListWidget::connect_activated_signal(
    const ActivatedSignal::slot_type& slot) const {
  return m_activated_signal.connect(slot);
}

connection ListWidget::connect_highlighted_signal(
    const HighlightedSignal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

connection ListWidget::connect_index_selected_signal(
    const IndexSelectedSignal::slot_type& slot) const {
  return m_index_selected_signal.connect(slot);
}

connection ListWidget::connect_value_selected_signal(
    const ValueSelectedSignal::slot_type& slot) const {
  return m_value_selected_signal.connect(slot);
}

QVariant ListWidget::get_value(unsigned int index) {
  if(static_cast<unsigned int>(m_layout->count() - 1) < index) {
    return QVariant();
  }
  return get_widget(index)->get_value();
}

void ListWidget::insert_item(DropDownItem* item) {
  m_item_selected_connections.push_back(std::move(
    item->connect_selected_signal(
      [=, index = m_layout->count()] (auto value) {
        on_item_selected(std::move(value), index);
      })));
  m_layout->insertWidget(m_layout->count(), item);
  update_height();
}

unsigned int ListWidget::item_count() const {
  return m_layout->count();
}

void ListWidget::remove_item(unsigned int index) {
  if(index > static_cast<unsigned int>(m_layout->count() - 1)) {
    return;
  }
  m_item_selected_connections.pop_back();
  for(auto i = std::size_t(index); i < m_item_selected_connections.size();
      ++i) {
    m_item_selected_connections[i] =
      std::move(get_widget(i + 1)->connect_selected_signal(
      [=] (auto value) {
        on_item_selected(std::move(value), i);
      }));
  }
  auto layout_item = m_layout->takeAt(index);
  delete layout_item->widget();
  delete layout_item;
  update_height();
}

bool ListWidget::set_highlight(const QString& text) {
  for(auto i = unsigned int(0); i < item_count(); ++i) {
    if(m_item_delegate.displayText(get_value(i)).startsWith(text,
        Qt::CaseInsensitive)) {
      set_highlight(i);
      return true;
    }
  }
  return false;
}

void ListWidget::set_items(std::vector<DropDownItem*> items) {
  while(auto item = m_layout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  m_item_selected_connections.clear();
  for(auto i = std::size_t(0); i < items.size(); ++i) {
    m_layout->addWidget(items[i]);
    m_item_selected_connections.push_back(std::move(
      items[i]->connect_selected_signal([=] (auto value) {
        on_item_selected(std::move(value), i);
      })));
  }
  if(m_layout->count() > 0) {
    update_height();
    static_cast<DropDownItem*>(m_layout->itemAt(0)->widget())->set_highlight();
    m_highlight_index = 0;
  }
}

DropDownItem* ListWidget::get_widget(unsigned int index) {
  return static_cast<DropDownItem*>(m_layout->itemAt(index)->widget());
}

void ListWidget::focus_next() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(0);
    return;
  }
  set_highlight((*m_highlight_index) + 1);
}

void ListWidget::focus_previous() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(m_layout->count() - 1);
    return;
  }
  set_highlight((*m_highlight_index) - 1);
}

void ListWidget::set_highlight(int index) {
  if(m_highlight_index) {
    get_widget(*m_highlight_index)->reset_highlight();
  }
  if(index < 0) {
    index = m_layout->count() - 1;
  } else if(index > m_layout->count() - 1) {
    index = 0;
  }
  m_highlight_index = index;
  auto highlighted_widget = get_widget(*m_highlight_index);
  highlighted_widget->set_highlight();
  scroll_to_highlight();
  m_activated_signal(highlighted_widget->get_value());
}

void ListWidget::scroll_to_highlight() {
  if(m_highlight_index != m_layout->count() - 1) {
    m_scroll_area->ensureWidgetVisible(get_widget(*m_highlight_index), 0,
      0);
  } else {
    m_scroll_area->verticalScrollBar()->setValue(
      m_scroll_area->verticalScrollBar()->maximum());
  }
}

void ListWidget::update_height() {
  if(m_layout->count() == 0) {
    return;
  }
  setFixedHeight(std::min(m_max_displayed_items, m_layout->count()) *
    m_layout->itemAt(0)->widget()->height() + BORDER_PADDING);
}

void ListWidget::on_item_selected(QVariant value, unsigned int index) {
  m_index_selected_signal(index);
  m_value_selected_signal(std::move(value));
}
