#include "Spire/Toolbar/ToolbarMenu.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

ToolbarMenu::ToolbarMenu(const QString& title, QWidget* parent)
    : StaticDropDownMenu({}, title, parent) {
  setDisabled(true);
}

void ToolbarMenu::add(const QString& text, const QImage& icon) {
  auto item = new DropDownItem(text, icon, this);
  item->setFixedHeight(scale_height(20));
  StaticDropDownMenu::insert_item(item);
  setEnabled(true);
}

void ToolbarMenu::remove_item(int index) {
  StaticDropDownMenu::remove_item(index);
  if(item_count() == 0) {
    setDisabled(true);
  }
}
