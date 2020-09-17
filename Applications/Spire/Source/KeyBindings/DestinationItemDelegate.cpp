#include "Spire/KeyBindings/DestinationItemDelegate.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

DestinationItemDelegate::DestinationItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* DestinationItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new FilteredDropDownMenu(
    {"One", "Two", "Three"}, static_cast<QWidget*>(this->parent()));
  editor->set_current_item(QString::fromStdString(
    index.data(Qt::DisplayRole).value<Region>().GetName()));
  editor->set_cell_style();
  connect(editor, &FilteredDropDownMenu::editingFinished,
    this, &DestinationItemDelegate::on_editing_finished);
  return editor;
}

void DestinationItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<FilteredDropDownMenu*>(
    editor)->get_item_or_invalid().value<QString>();
  auto region = [&] {
    if(item.isEmpty() && !index.data().value<Region>().IsGlobal()) {
      return Region();
    }
    return Region::Global();
  }();
  region.SetName(item.toStdString());
  model->setData(index, QVariant::fromValue<Region>(region), Qt::DisplayRole);
}
