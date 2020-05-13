#include "Spire/Ui/DropdownMenuList.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Spire/Utility.hpp"
#include "Spire/Ui/DropdownMenuItem.hpp"
#include "Spire/Ui/DropShadow.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto MAX_VISIBLE_ITEMS = 5;
}

DropDownMenuList::DropDownMenuList(
    const std::vector<QString>& items, QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_highlight_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = new DropShadow(true, false, this);
  setFixedHeight(1 + scale_height(20) * items.size());
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  m_scroll_area = new ScrollArea(false, this);
  m_scroll_area->setWidgetResizable(true);
  m_scroll_area->setFocusProxy(parent);
  m_scroll_area->setObjectName("dropdown_menu_list_scroll_area");
  layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  set_items(items);
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  m_scroll_area->setWidget(m_list_widget);
  parent->installEventFilter(this);
}

void DropDownMenuList::set_items(const std::vector<QString>& items) {
  while(auto item = m_list_widget->layout()->takeAt(0)) {
    delete item->widget();
    delete item;
  }
  setFixedHeight(scale_height(20) *
    std::min(static_cast<int>(items.size()), MAX_VISIBLE_ITEMS));
  for(auto& item : items) {
    auto menu_item = new DropDownMenuItem(item, m_list_widget);
    menu_item->connect_selected_signal([=] (auto& t) { on_select(t); });
    m_list_widget->layout()->addWidget(menu_item);
  }
}

QString DropDownMenuList::get_next(const QString& text) {
  auto num_items = m_list_widget->layout()->count();
  auto index = (get_index(text) + 1) % num_items;
  return static_cast<DropDownMenuItem*>(
    m_list_widget->layout()->itemAt(index)->widget())->text();
}

QString DropDownMenuList::get_previous(const QString& text) {
  auto num_items = m_list_widget->layout()->count();
  auto index = (get_index(text) - 1) % num_items;
  if(index < 0) {
    index = num_items - 1;
  }
  return static_cast<DropDownMenuItem*>(
    m_list_widget->layout()->itemAt(index)->widget())->text();
}

connection DropDownMenuList::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool DropDownMenuList::eventFilter(QObject* object, QEvent* event) {
  if(object == parent()) {
    if(event->type() == QEvent::KeyPress && isVisible()) {
      auto key_event = static_cast<QKeyEvent*>(event);
      if(key_event->key() == Qt::Key_Down) {
        focus_next();
        return true;
      } else if(key_event->key() == Qt::Key_Up) {
        focus_previous();
        return true;
      } else if(key_event->key() == Qt::Key_Enter ||
          key_event->key() == Qt::Key_Return) {
        if(m_highlight_index >= 0) {
          if(auto widget = m_list_widget->layout()->itemAt(m_highlight_index)) {
            on_select(static_cast<DropDownMenuItem*>(
              widget->widget())->text());
          }
          return true;
        }
      } else if(key_event->key() == Qt::Key_Escape) {
        close();
      }
    }
  }
  return false;
}

void DropDownMenuList::showEvent(QShowEvent* event) {
  m_highlight_index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto widget = static_cast<DropDownMenuItem*>(
      m_list_widget->layout()->itemAt(i)->widget());
    widget->remove_highlight();
    widget->update();
  }
}

int DropDownMenuList::get_index(const QString& text) {
  auto index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto& item_text = static_cast<DropDownMenuItem*>(
      m_list_widget->layout()->itemAt(i)->widget())->text();
    if(item_text == text) {
      index = i;
      break;
    }
  }
  return index;
}

void DropDownMenuList::on_select(const QString& text) {
  m_selected_signal(text);
}

void DropDownMenuList::focus_next() {
  auto index = (m_highlight_index + 1) % m_list_widget->layout()->count();
  if(m_highlight_index < 0) {
    m_highlight_index = 0;
  }
  update_highlights(m_highlight_index, index);
  m_highlight_index = index;
}

void DropDownMenuList::focus_previous() {
  if(m_highlight_index < 0 &&
      m_highlight_index > m_list_widget->layout()->count() - 1) {
    m_highlight_index = 0;
  }
  auto index = m_highlight_index - 1;
  if(index < 0) {
    index = m_list_widget->layout()->count() - 1;
  }
  update_highlights(m_highlight_index, index);
  m_highlight_index = index;
}

void DropDownMenuList::update_highlights(int old_index, int new_index) {
  if(auto previous_widget = m_list_widget->layout()->itemAt(old_index)) {
    static_cast<DropDownMenuItem*>(
      previous_widget->widget())->remove_highlight();
    previous_widget->widget()->update();
  }
  if(auto current_widget = m_list_widget->layout()->itemAt(new_index)) {
    static_cast<DropDownMenuItem*>(current_widget->widget())->set_highlight();
    current_widget->widget()->update();
    m_scroll_area->ensureWidgetVisible(current_widget->widget());
  }
}
