#include "Spire/Ui/DropDownList.hpp"
#include <QKeyEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto BORDER_PADDING = 2;
}

DropDownList::DropDownList(std::vector<DropDownItem*> items,
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
  set_widget(m_scroll_area);
  set_items(items);
  parent->installEventFilter(this);
}

bool DropDownList::eventFilter(QObject* watched, QEvent* event) {
  if(watched == parent()) {
    if(event->type() == QEvent::KeyPress) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Up) {
        focus_previous();
        return true;
      } else if(e->key() == Qt::Key_Down) {
        focus_next();
        return true;
      } else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
        if(isVisible() && m_highlight_index) {
          m_selected_signal(get_widget(*m_highlight_index)->get_value());
        }
      }
    }
  }
  return DropDownWindow::eventFilter(watched, event);
}

void DropDownList::hideEvent(QHideEvent* event) {
  m_scroll_area->verticalScrollBar()->setValue(0);
  if(m_highlight_index) {
    get_widget(*m_highlight_index)->reset_highlight();
    m_highlight_index = boost::none;
  }
}

void DropDownList::keyPressEvent(QKeyEvent* event) {
  if(event->key() == Qt::Key_Up) {
    focus_previous();
  } else if(event->key() == Qt::Key_Down) {
    focus_next();
  } else if(event->key() == Qt::Key_Escape) {
    hide();
  } else if(event->key() == Qt::Key_Enter) {
    if(m_highlight_index) {
      m_selected_signal(get_widget(*m_highlight_index)->get_value());
      hide();
    }
  }
  DropDownWindow::keyPressEvent(event);
}

connection DropDownList::connect_activated_signal(
    const ActivatedSignal::slot_type& slot) const {
  return m_activated_signal.connect(slot);
}

connection DropDownList::connect_highlighted_signal(
    const HighlightedSignal::slot_type& slot) const {
  return m_highlighted_signal.connect(slot);
}

connection DropDownList::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

QVariant DropDownList::get_value(int index) {
  if(m_layout->count() - 1 < index) {
    return QVariant();
  }
  return get_widget(index)->get_value();
}

void DropDownList::set_items(std::vector<DropDownItem*> items) {
  while(auto item = m_layout->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  for(auto item : items) {
    m_layout->addWidget(item);
    item->connect_selected_signal([=] (const auto& value) {
      on_item_selected(value);
    });
  }
  if(m_layout->count() > 0) {
    setFixedHeight(std::min(m_max_displayed_items, m_layout->count()) *
      m_layout->itemAt(0)->widget()->height() + BORDER_PADDING);
  } else {
    hide();
  }
}

DropDownItem* DropDownList::get_widget(int index) {
  return static_cast<DropDownItem*>(m_layout->itemAt(index)->widget());
}

void DropDownList::focus_next() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(0);
    return;
  }
  set_highlight((*m_highlight_index) + 1);
}

void DropDownList::focus_previous() {
  if(m_layout->count() == 0) {
    return;
  }
  if(!m_highlight_index) {
    set_highlight(m_layout->count() - 1);
    return;
  }
  set_highlight((*m_highlight_index) - 1);
}

void DropDownList::set_highlight(int index) {
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

void DropDownList::scroll_to_highlight() {
  if(m_highlight_index != m_layout->count() - 1) {
    m_scroll_area->ensureWidgetVisible(get_widget(*m_highlight_index), 0,
      0);
  } else {
    m_scroll_area->verticalScrollBar()->setValue(
      m_scroll_area->verticalScrollBar()->maximum());
  }
}

void DropDownList::on_item_selected(const QVariant& value) {
  m_selected_signal(value);
  hide();
}
