#include "Spire/Charting/StyleDropDownMenuList.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "Spire/Charting/StyleDropDownMenuItem.hpp"
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/DropShadow.hpp"

using namespace boost::signals2;
using namespace Spire;

StyleDropDownMenuList::StyleDropDownMenuList(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_highlight_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  m_shadow = std::make_unique<DropShadow>(true, true, this);
  setStyleSheet("background-color: #C8C8C8;");
  setFixedHeight(scale_height(120));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({scale_width(1), 0, scale_width(1),
    scale_height(1)});
  m_list_widget = new QWidget(this);
  m_list_widget->setFixedHeight(height());
  layout->addWidget(m_list_widget);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  auto solid = new StyleDropDownMenuItem(TrendLineStyle::SOLID,
    m_list_widget);
  solid->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(solid);
  auto solid_wide = new StyleDropDownMenuItem(TrendLineStyle::SOLID_WIDE,
    m_list_widget);
  solid_wide->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(solid_wide);
  auto dashed = new StyleDropDownMenuItem(TrendLineStyle::DASHED,
    m_list_widget);
  dashed->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dashed);
  auto dashed_wide = new StyleDropDownMenuItem(TrendLineStyle::DASHED_WIDE,
    m_list_widget);
  dashed_wide->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dashed_wide);
  auto dotted = new StyleDropDownMenuItem(TrendLineStyle::DOTTED,
    m_list_widget);
  dotted->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dotted);
  auto dotted_wide = new StyleDropDownMenuItem(TrendLineStyle::DOTTED_WIDE,
    m_list_widget);
  dotted_wide->connect_selected_signal([=] (auto t) { on_select(t); });
  list_layout->addWidget(dotted_wide);
  parent->installEventFilter(this);
}

TrendLineStyle StyleDropDownMenuList::get_next(TrendLineStyle style) {
  auto num_items = m_list_widget->layout()->count();
  auto index = (get_index(style) + 1) % num_items;
  return static_cast<StyleDropDownMenuItem*>(
    m_list_widget->layout()->itemAt(index)->widget())->get_style();
}

TrendLineStyle StyleDropDownMenuList::get_previous(TrendLineStyle style) {
  auto num_items = m_list_widget->layout()->count();
  auto index = (get_index(style) - 1) % num_items;
  if(index < 0) {
    index = num_items - 1;
  }
  return static_cast<StyleDropDownMenuItem*>(
    m_list_widget->layout()->itemAt(index)->widget())->get_style();
}

connection StyleDropDownMenuList::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool StyleDropDownMenuList::eventFilter(QObject* object, QEvent* event) {
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
        on_select(static_cast<StyleDropDownMenuItem*>(m_list_widget->layout()->
          itemAt(m_highlight_index)->widget())->get_style());
        return true;
      } else if(key_event->key() == Qt::Key_Escape) {
        close();
      }
    }
  }
  return false;
}

void StyleDropDownMenuList::showEvent(QShowEvent* event) {
  m_highlight_index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto widget = static_cast<StyleDropDownMenuItem*>(
      m_list_widget->layout()->itemAt(i)->widget());
    widget->remove_highlight();
    widget->update();
  }
}

int StyleDropDownMenuList::get_index(TrendLineStyle style) {
  auto index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto item_style = static_cast<StyleDropDownMenuItem*>(
      m_list_widget->layout()->itemAt(i)->widget())->get_style();
    if(item_style == style) {
      index = i;
      break;
    }
  }
  return index;
}

void StyleDropDownMenuList::on_select(TrendLineStyle style) {
  m_selected_signal(style);
}

void StyleDropDownMenuList::focus_next() {
  auto index = (m_highlight_index + 1) % m_list_widget->layout()->count();
  if(m_highlight_index < 0) {
    m_highlight_index = 0;
  }
  update_highlights(m_highlight_index, index);
  m_highlight_index = index;
}

void StyleDropDownMenuList::focus_previous() {
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

void StyleDropDownMenuList::update_highlights(int old_index, int new_index) {
  auto previous_widget = m_list_widget->layout()->itemAt(old_index)->widget();
  static_cast<StyleDropDownMenuItem*>(previous_widget)->remove_highlight();
  previous_widget->update();
  auto current_widget = m_list_widget->layout()->itemAt(new_index)->widget();
  static_cast<StyleDropDownMenuItem*>(current_widget)->set_highlight();
  current_widget->update();
}
