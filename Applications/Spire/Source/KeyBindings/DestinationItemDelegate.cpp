#include "Spire/KeyBindings/DestinationItemDelegate.hpp"
#include "Nexus/Definitions/Region.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

DestinationItemDelegate::DestinationItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* DestinationItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new InputFieldEditor(
    QString::fromStdString(index.data().value<Region>().GetName()),
    {"One", "Two", "Three"}, static_cast<QWidget*>(this->parent()));
  connect(editor, &InputFieldEditor::editingFinished,
    this, &DestinationItemDelegate::on_editing_finished);
  return editor;
}

void DestinationItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item();
  auto region = Region::Global();
  region.SetName(item.toStdString());
  model->setData(index, QVariant::fromValue<Region>(region), Qt::DisplayRole);
}
