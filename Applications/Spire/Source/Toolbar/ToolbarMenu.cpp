#include "Spire/Toolbar/ToolbarMenu.hpp"
#include "Spire/Spire/Dimensions.hpp"
// TODO: delete this class/file entirely?
#include "Spire/Toolbar/MenuIconSizeProxyStyle.hpp"

using namespace boost::signals2;
using namespace Spire;

ToolbarMenu::ToolbarMenu(const QString& title, QWidget* parent)
    : StaticDropDownMenu({}, title, parent) {
  connect_selected_signal([=] (const auto& item) { on_item_selected(item); });
  setDisabled(true);
}

void ToolbarMenu::add(const QString& text, const QImage& icon) {
  auto item = new DropDownItem(text, icon, this);
  item->setFixedHeight(scale_height(20));
  insert_item(item);
  // TODO: Using QString for keys results in collisions when window names
  //      are the same. Consult on whether duplicates should be removed, or
  //      if another method should be used to store indexes.
  m_item_to_index[text] = m_item_to_index.size();
  setEnabled(true);
}

void ToolbarMenu::remove(int index) {
  remove_item(index);
  auto removed_item = [&] () {
    auto iter = QMutableHashIterator<QString, int>(m_item_to_index);
    while(iter.hasNext()) {
      iter.next();
      if(iter.value() == index) {
        return iter.key();
      }
    }
    return QString();
  }();
  m_item_to_index.remove(removed_item);
  auto iter = QMutableHashIterator<QString, int>(m_item_to_index);
  while(iter.hasNext()) {
    iter.next();
    if(iter.value() > index) {
      iter.setValue(iter.value() - 1);
    }
  }
  if(m_item_to_index.size() == 0) {
    setDisabled(true);
  }
}

connection ToolbarMenu::connect_item_selected_signal(
    const ItemSelectedSignal::slot_type& slot) const {
  return m_item_selected_signal.connect(slot);
}

void ToolbarMenu::on_item_selected(const QVariant& item) {
  auto index = m_item_to_index[item.value<QString>()];
  m_item_selected_signal(index);
}
