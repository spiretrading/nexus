#include "Spire/Ui/FilteredDropDownMenu.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

FilteredDropDownMenu::FilteredDropDownMenu(const std::vector<QVariant>& items,
    QWidget* parent)
    : QLineEdit(parent) {
  setFocusPolicy(Qt::StrongFocus);
  if(!items.empty()) {
    m_current_item = items.front();
  }
  m_menu_list = new DropDownList({}, true, this);
  m_menu_list->connect_selected_signal([=] (const auto& value) {
    on_item_selected(value);
  });
  set_items(items);
}

void FilteredDropDownMenu::paintEvent(QPaintEvent* event) {
  
}

connection FilteredDropDownMenu::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void FilteredDropDownMenu::set_items(const std::vector<QVariant>& items) {
  auto widget_items = std::vector<DropDownItem*>();
  widget_items.reserve(items.size());
  for(auto& item : items) {
    auto item_widget = new DropDownItem(item, this);
    item_widget->setFixedHeight(scale_height(20));
    widget_items.push_back(item_widget);
  }
  m_menu_list->set_items(widget_items);
  m_menu_list->setFixedWidth(width());
}
