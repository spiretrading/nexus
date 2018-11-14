#include "spire/ui/dropdown_menu_list.hpp"
#include <QKeyEvent>
#include <QVBoxLayout>
#include "spire/spire/dimensions.hpp"
#include "spire/ui/dropdown_menu_item.hpp"
#include "spire/ui/drop_shadow.hpp"

using namespace boost::signals2;
using namespace Spire;

DropdownMenuList::DropdownMenuList(
    const std::initializer_list<QString>& items,
    QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_highlight_index(-1) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  setAttribute(Qt::WA_TranslucentBackground);
  m_shadow = std::make_unique<DropShadow>(false, this);
  setFixedHeight(1 + scale_height(20) * items.size());
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins({});
  m_scroll_area = new QScrollArea(this);
  m_scroll_area->setWidgetResizable(true);
  m_scroll_area->setObjectName("dropdown_menu_list_scroll_area");
  m_scroll_area->setFrameShape(QFrame::NoFrame);
  m_scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_scroll_area->setStyleSheet(QString(R"(
    #dropdown_menu_list_scroll_area {
      background-color: #FFFFFF;
      border-bottom: %2px solid #A0A0A0;
      border-left: %2px solid #A0A0A0;
      border-right: %2px solid #A0A0A0;
      border-top: none;
    }
    
    QScrollBar {
      background-color: #FFFFFF;
      border: none;
    }

    QScrollBar::handle:vertical {
      background-color: #EBEBEB;
      margin-left: %3px;
      width: %1px;
    }

    QScrollBar::sub-line:vertical {
      background: none;
      border: none;
    }

    QScrollBar::add-line:vertical {
      background: none;
      border: none;
    })").arg(scale_width(15)).arg(scale_width(1)).arg(scale_width(2)));
  layout->addWidget(m_scroll_area);
  m_list_widget = new QWidget(m_scroll_area);
  auto list_layout = new QVBoxLayout(m_list_widget);
  list_layout->setContentsMargins({});
  list_layout->setSpacing(0);
  for(auto& item : items) {
    auto i = new DropdownMenuItem(item, m_list_widget);
    i->connect_selected_signal([=] (auto& t) { on_select(t); });
    list_layout->addWidget(i);
  }
  m_list_widget->setStyleSheet("background-color: #FFFFFF;");
  m_scroll_area->setWidget(m_list_widget);
  parent->installEventFilter(this);
}

connection DropdownMenuList::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

bool DropdownMenuList::eventFilter(QObject* object, QEvent* event) {
  if(object == parent()) {
    if(event->type() == QEvent::KeyPress && isVisible()) {
      auto e = static_cast<QKeyEvent*>(event);
      if(e->key() == Qt::Key_Tab || e->key() == Qt::Key_Down) {
        focus_next();
        return true;
      } else if((e->key() & Qt::Key_Tab &&
          e->modifiers() & Qt::ShiftModifier) || e->key() == Qt::Key_Up) {
        focus_previous();
        return true;
      } else if(e->key() == Qt::Key_Escape) {
        close();
      }
    }
  }
  return false;
}

void DropdownMenuList::showEvent(QShowEvent* event) {
  m_highlight_index = -1;
  for(auto i = 0; i < m_list_widget->layout()->count(); ++i) {
    auto w = m_list_widget->layout()->itemAt(i)->widget();
    static_cast<DropdownMenuItem*>(w)->remove_highlight();
    w->update();
  }
}

void DropdownMenuList::on_select(const QString& text) {
  m_selected_signal(text);
}

void DropdownMenuList::focus_next() {
  auto index = (m_highlight_index + 1) % m_list_widget->layout()->count();
  if(m_highlight_index < 0) {
    m_highlight_index = 0;
  }
  update_highlights(m_highlight_index, index);
  m_highlight_index = index;
}

void DropdownMenuList::focus_previous() {
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

void DropdownMenuList::update_highlights(int old_index, int new_index) {
  auto previous_widget = m_list_widget->layout()->itemAt(old_index)->widget();
  static_cast<DropdownMenuItem*>(previous_widget)->remove_highlight();
  previous_widget->update();
  auto current_widget = m_list_widget->layout()->
    itemAt(new_index)->widget();
  static_cast<DropdownMenuItem*>(current_widget)->set_highlight();
  current_widget->update();
}
