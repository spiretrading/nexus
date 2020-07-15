#include "Spire/Ui/FilteredDropDownMenu.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

FilteredDropDownMenu::FilteredDropDownMenu(const std::vector<QVariant>& items,
    QWidget* parent)
    : QLineEdit(parent) {
  setFocusPolicy(Qt::StrongFocus);
  apply_line_edit_style(this);
  connect(this, &QLineEdit::textEdited, this,
    &FilteredDropDownMenu::on_text_edited);
  if(!items.empty()) {
    m_current_item = items.front();
  }
  m_menu_list = new DropDownList({}, true, this);
  m_menu_list->connect_activated_signal([=] (const auto& value) {
    on_item_activated(value);
  });
  m_menu_list->connect_selected_signal([=] (const auto& value) {
    on_item_selected(value);
  });
  set_items(items);
}

connection FilteredDropDownMenu::connect_selected_signal(
    const SelectedSignal::slot_type& slot) const {
  return m_selected_signal.connect(slot);
}

void FilteredDropDownMenu::set_items(const std::vector<QVariant>& items) {
  m_items = std::move(items);
  m_menu_list->set_items(std::move(create_widget_items(m_items)));
  m_menu_list->setFixedWidth(width());
}

const std::vector<DropDownItem*> FilteredDropDownMenu::create_widget_items(
    const std::vector<QVariant>& items) {
  return create_widget_items(items, "");
}

const std::vector<DropDownItem*> FilteredDropDownMenu::create_widget_items(
    const std::vector<QVariant>& items, const QString& filter_text) {
  auto widget_items = std::vector<DropDownItem*>();
  widget_items.reserve(items.size());
  for(const auto& item : items) {
    if(filter_text.isEmpty() || m_item_delegate.displayText(item).startsWith(
        filter_text, Qt::CaseInsensitive)) {
      auto item_widget = new DropDownItem(item, this);
      item_widget->setFixedHeight(scale_height(20));
      item_widget->connect_selected_signal([=] (const auto& item) {
        on_item_selected(item);
      });
      widget_items.push_back(item_widget);
    }
  }
  return widget_items;
}

void FilteredDropDownMenu::on_item_activated(const QVariant& item) {
  setText(m_item_delegate.displayText(item));
}

void FilteredDropDownMenu::on_item_selected(const QVariant& item) {
  m_selected_signal(item);
}

void FilteredDropDownMenu::on_text_edited(const QString& text) {
  if(text.isEmpty()) {
    m_menu_list->set_items(std::move(create_widget_items(m_items)));
    m_menu_list->show();
    return;
  }
  auto items = std::move(create_widget_items(m_items, text));
  if(items.empty()) {
    m_menu_list->hide();
    return;
  }
  m_menu_list->set_items(items);
  m_menu_list->show();
}
