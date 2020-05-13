#include "Spire/KeyBindings/DestinationItemDelegate.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Spire;

DestinationItemDelegate::DestinationItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* DestinationItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new InputFieldEditor(index.data().value<QString>(),
    {"One", "Two", "Three"}, static_cast<QWidget*>(this->parent()));
  connect(editor, &InputFieldEditor::editingFinished,
    this, &DestinationItemDelegate::on_editing_finished);
  return editor;
}

void DestinationItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item();
  model->setData(index, item, Qt::DisplayRole);
}
