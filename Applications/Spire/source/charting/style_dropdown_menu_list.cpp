#include "spire/charting/style_dropdown_menu_list.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace boost::signals2;
using namespace Spire;

StyleDropdownMenuList::StyleDropdownMenuList(QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_highlight_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  //setAttribute(Qt::WA_TranslucentBackground);
  //m_shadow = std::make_unique<DropShadow>(false, this);
  setFixedWidth(scale_width(70));
  setStyleSheet(QString(R"(
      background-color: #FFFFFF;
      border-bottom: %1px solid #A0A0A0;
      border-left: %2px solid #A0A0A0;
      border-right: %2px solid #A0A0A0;
      border-top: none;
    )").arg(scale_height(1)).arg(scale_width(1)));
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  layout->setSpacing(0);
  // TODO: temporary height, add list items
  parent->installEventFilter(this);
}

TrendLineStyle StyleDropdownMenuList::get_next(TrendLineStyle style) {
  //auto num_items = m_list_widget->layout()->count();
  //auto index = (get_index(text) + 1) % num_items;
  //return static_cast<DropdownMenuItem*>(
  //  m_list_widget->layout()->itemAt(index)->widget())->text();
  return TrendLineStyle::DASHED;
}

TrendLineStyle StyleDropdownMenuList::get_previous(TrendLineStyle style) {
  //auto num_items = m_list_widget->layout()->count();
  //auto index = (get_index(text) - 1) % num_items;
  //if(index < 0) {
  //  index = num_items - 1;
  //}
  //return static_cast<DropdownMenuItem*>(
  //  m_list_widget->layout()->itemAt(index)->widget())->text();
  return TrendLineStyle::DOTTED;
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
        //focus_next();
        return true;
      } else if((key_event->key() & Qt::Key_Tab &&
          key_event->modifiers() & Qt::ShiftModifier) ||
          key_event->key() == Qt::Key_Up) {
        //focus_previous();
        return true;
      } else if(key_event->key() == Qt::Key_Enter ||
          key_event->key() == Qt::Key_Return) {
        //on_select(static_cast<DropdownMenuItem*>(m_list_widget->layout()->
        //  itemAt(m_highlight_index)->widget())->text());
        return true;
      } else if(key_event->key() == Qt::Key_Escape) {
        close();
      }
    }
  }
  return false;
}

void StyleDropdownMenuList::showEvent(QShowEvent* event) {
  //m_highlight_index = -1;
  //for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
  //  auto widget = static_cast<DropdownMenuItem*>(
  //    m_list_widget->layout()->itemAt(i)->widget());
  //  widget->remove_highlight();
  //  widget->update();
  //}
}
