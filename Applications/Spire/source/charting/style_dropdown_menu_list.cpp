#include "spire/charting/style_dropdown_menu_list.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "spire/charting/style_dropdown_menu_item.hpp"
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace boost::signals2;
using namespace Spire;

StyleDropdownMenuList::StyleDropdownMenuList(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_highlight_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<DropShadow>(false, this);
  setFixedHeight(scale_height(120));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  m_list_widget = new QWidget(this);
  m_list_widget->setFixedHeight(height());
  layout->addWidget(m_list_widget);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  auto solid = new StyleDropdownMenuItem(TrendLineStyle::SOLID,
    m_list_widget);
  solid->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(solid);
  auto solid_wide = new StyleDropdownMenuItem(TrendLineStyle::SOLID_WIDE,
    m_list_widget);
  solid_wide->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(solid_wide);
  auto dashed = new StyleDropdownMenuItem(TrendLineStyle::DASHED,
    m_list_widget);
  dashed->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dashed);
  auto dashed_wide = new StyleDropdownMenuItem(TrendLineStyle::DASHED_WIDE,
    m_list_widget);
  dashed_wide->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dashed_wide);
  auto dotted = new StyleDropdownMenuItem(TrendLineStyle::DOTTED,
    m_list_widget);
  dotted->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dotted);
  auto dotted_wide = new StyleDropdownMenuItem(TrendLineStyle::DOTTED_WIDE,
    m_list_widget);
  dotted_wide->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dotted_wide);
  parent->installEventFilter(this);
}

TrendLineStyle StyleDropdownMenuList::get_next(TrendLineStyle style) {
  auto num_items = m_list_widget->layout()->count();
  auto index = (get_index(style) + 1) % num_items;
  return static_cast<StyleDropdownMenuItem*>(
    m_list_widget->layout()->itemAt(index)->widget())->get_style();
}

TrendLineStyle StyleDropdownMenuList::get_previous(TrendLineStyle style) {
  auto num_items = m_list_widget->layout()->count();
  auto index = (get_index(style) - 1) % num_items;
  if(index < 0) {
    index = num_items - 1;
  }
  return static_cast<StyleDropdownMenuItem*>(
    m_list_widget->layout()->itemAt(index)->widget())->get_style();
}

connection StyleDropdownMenuList::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool StyleDropdownMenuList::eventFilter(QObject* object, QEvent* event) {
  if(object == parent()) {
    if(event->type() == QEvent::KeyPress && isVisible()) {
      auto key_event = static_cast<QKeyEvent*>(event);
      if(key_event->key() == Qt::Key_Tab || key_event->key() == Qt::Key_Down) {
        focus_next();
        return true;
      } else if((key_event->key() & Qt::Key_Tab &&
          key_event->modifiers() & Qt::ShiftModifier) ||
          key_event->key() == Qt::Key_Up) {
        focus_previous();
        return true;
      } else if(key_event->key() == Qt::Key_Enter ||
          key_event->key() == Qt::Key_Return) {
        on_select(static_cast<StyleDropdownMenuItem*>(m_list_widget->layout()->
          itemAt(m_highlight_index)->widget())->get_style());
        return true;
      } else if(key_event->key() == Qt::Key_Escape) {
        close();
      }
    }
  }
  return false;
}

void StyleDropdownMenuList::showEvent(QShowEvent* event) {
  m_highlight_index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto widget = static_cast<StyleDropdownMenuItem*>(
      m_list_widget->layout()->itemAt(i)->widget());
    widget->remove_highlight();
    widget->update();
  }
}

int StyleDropdownMenuList::get_index(TrendLineStyle style) {
  auto index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto item_style = static_cast<StyleDropdownMenuItem*>(
      m_list_widget->layout()->itemAt(i)->widget())->get_style();
    if(item_style == style) {
      index = i;
      break;
    }
  }
  return index;
}

void StyleDropdownMenuList::on_select(TrendLineStyle style) {
  m_selected_signal(style);
}

void StyleDropdownMenuList::focus_next() {
  auto index = (m_highlight_index + 1) % m_list_widget->layout()->count();
  if(m_highlight_index < 0) {
    m_highlight_index = 0;
  }
  update_highlights(m_highlight_index, index);
  m_highlight_index = index;
}

void StyleDropdownMenuList::focus_previous() {
  if(m_highlight_index < 0) {
    m_highlight_index = 0;
  }
  auto index = m_highlight_index - 1;
  if(index < 0) {
    index = m_list_widget->layout()->count() - 1;
  }
  update_highlights(m_highlight_index, index);
  m_highlight_index = index;
}

void StyleDropdownMenuList::update_highlights(int old_index, int new_index) {
  auto previous_widget = m_list_widget->layout()->itemAt(old_index)->widget();
  static_cast<StyleDropdownMenuItem*>(previous_widget)->remove_highlight();
  previous_widget->update();
  auto current_widget = m_list_widget->layout()->itemAt(new_index)->widget();
  static_cast<StyleDropdownMenuItem*>(current_widget)->set_highlight();
  current_widget->update();
}
