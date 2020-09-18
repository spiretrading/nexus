#include "Spire/KeyBindings/SideItemDelegate.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/FilteredDropDownMenu.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideItemDelegate::SideItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* SideItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return Spire::displayText(data.value<Side>());
    }
    return QString();
  }();
  auto editor = new FilteredDropDownMenu(
    {Spire::displayText(Side::ASK), Spire::displayText(Side::BID)},
    static_cast<QWidget*>(this->parent()));
  editor->set_current_item(current_data);
  editor->set_cell_style();
  connect(editor, &FilteredDropDownMenu::editingFinished,
    this, &SideItemDelegate::on_editing_finished);
  return editor;
}

void SideItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto variant = [&] {
    auto item = static_cast<FilteredDropDownMenu*>(
      editor)->get_item_or_invalid().value<QString>().toLower();
    if(item == Spire::displayText(Side::BID).toLower()) {
      return QVariant::fromValue<Side>(Side::BID);
    } else if(item == Spire::displayText(Side::ASK).toLower()) {
      return QVariant::fromValue<Side>(Side::ASK);
    }
    return QVariant();
  }();
  model->setData(index, variant, Qt::DisplayRole);
}
