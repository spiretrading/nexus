#include "Spire/Ui/DropDownMenu2.hpp"
#include <QFocusEvent>
#include <QScrollBar>
#include <QVBoxLayout>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/ScrollArea.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

namespace {
  const auto MAX_DISPLAYED_ITEMS = 5;

  auto BORDER_PADDING() {
    static auto padding = 2 * scale_width(1);
    return padding;
  }

  void adjust_current_index(DropDownMenu2& menu, int amount) {
    if(menu.get_count() == 0 || amount == 0) {
      return;
    }
    auto last_index = menu.get_count() - 1;
    auto index = [&] {
      if(auto index = menu.get_current()) {
        *index += amount;
        if(index > last_index) {
          return 0;
        } else if(index < 0) {
          return last_index;
        }
        return *index;
      } else if(amount > 0) {
        return 0;
      }
      return last_index;
    }();
    menu.set_current(index);
  }
}

DropDownMenu2::DropDownMenu2(std::vector<DropDownMenuItem2*> items,
    QWidget* parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_selected_index(0) {
  setAttribute(Qt::WA_ShowWithoutActivating);
  m_shadow = new DropShadow(true, false, this);
  setObjectName("DropDownMenu");
  setStyleSheet(R"(
    #DropDownMenu {
      background-color: #C8C8C8;
    })");
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(scale_width(1), 0, scale_width(1),
    scale_height(1));
  m_scroll_area = new ScrollArea(this);
  m_scroll_area->setFocusProxy(parent);
  m_scroll_area->setWidgetResizable(true);
  layout->addWidget(m_scroll_area);
  auto body = new QWidget(m_scroll_area);
  m_list_layout = new QVBoxLayout(body);
  m_list_layout->setContentsMargins({});
  m_list_layout->setSpacing(0);
  m_scroll_area->setWidget(body);
  for(auto& item : items) {
    m_list_layout->addWidget(item);
    m_item_hovered_connections.AddConnection(item->connect_hovered_signal(
      [=] { m_hovered_signal(item->get_value()); }));
    m_item_selected_connections.AddConnection(item->connect_selected_signal(
      [=] { on_item_selected(item->get_value(), item); }));
  }
  if(m_list_layout->count() > 0) {
    update_height();
    static_cast<DropDownMenuItem2*>(
      m_list_layout->itemAt(0)->widget())->set_highlighted();
    m_selected_index = 0;
    m_current_index = 0;
  }
}

void DropDownMenu2::keyPressEvent(QKeyEvent* event) {
  switch(event->key()) {
    case Qt::Key_Down:
      increment_current(*this);
      break;
    case Qt::Key_Up:
      decrement_current(*this);
      break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Space:
      on_item_selected(get_value(*m_current_index), *m_current_index);
      break;
    case Qt::Key_Escape:
      hide();
      break;
  }
}

const QVariant& DropDownMenu2::get_value(int index) const {
  return get_item(index)->get_value();
}

boost::optional<int> DropDownMenu2::get_current() const {
  return m_current_index;
}

void DropDownMenu2::set_current(int index) {
  if(m_current_index) {
    get_item(*m_current_index)->reset_highlighted();
  }
  m_current_index = index;
  auto highlighted_widget = get_item(*m_current_index);
  highlighted_widget->set_highlighted();
  scroll_to_current_index();
  m_current_signal(highlighted_widget->get_value());
}

boost::optional<int> DropDownMenu2::get_selected() const {
  return m_selected_index;
}

void DropDownMenu2::set_selected(int index) {
  on_item_selected(get_value(index), index);
}

int DropDownMenu2::get_count() const {
  return m_list_layout->count();
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

DropDownMenuItem2* DropDownMenu2::get_item(int index) const {
  return static_cast<DropDownMenuItem2*>(
    m_list_layout->itemAt(index)->widget());
}

void DropDownMenu2::scroll_to_current_index() {
  if(m_current_index != m_list_layout->count() - 1) {
    m_scroll_area->ensureWidgetVisible(get_item(*m_current_index), 0, 0);
  } else {
    m_scroll_area->verticalScrollBar()->setValue(
      m_scroll_area->verticalScrollBar()->maximum());
  }
}

void DropDownMenu2::update_height() {
  if(m_list_layout->count() == 0) {
    return;
  }
  setFixedHeight(std::min(MAX_DISPLAYED_ITEMS, m_list_layout->count()) *
    m_list_layout->itemAt(0)->widget()->height() + BORDER_PADDING());
  m_scroll_area->setFixedHeight(height() - 1);
}

void DropDownMenu2::on_item_selected(const QVariant& value, int index) {
  m_current_index = index;
  m_selected_index = index;
  hide();
  m_current_signal(value);
  m_selected_signal(value);
}

void DropDownMenu2::on_item_selected(const QVariant& value,
    DropDownMenuItem2* item) {
  on_item_selected(value, m_list_layout->indexOf(item));
}

void Spire::decrement_current(DropDownMenu2& menu) {
  adjust_current_index(menu, -1);
}

void Spire::increment_current(DropDownMenu2& menu) {
  adjust_current_index(menu, 1);
}

void Spire::set_current(DropDownMenu2& menu, const QString& text) {
  static const auto item_delegate = CustomVariantItemDelegate();
  for(auto i = 0; i < menu.get_count(); ++i) {
    if(item_delegate.displayText(menu.get_value(i)).startsWith(text,
        Qt::CaseInsensitive)) {
      menu.set_current(i);
      break;
    }
  }
}
