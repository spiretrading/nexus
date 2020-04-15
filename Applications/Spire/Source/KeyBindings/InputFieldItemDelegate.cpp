#include "Spire/KeyBindings/InputFieldItemDelegate.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Spire;

InputFieldItemDelegate::InputFieldItemDelegate(std::vector<QString> items,
  QWidget* parent)
  : QStyledItemDelegate(parent),
    m_items(std::move(items)) {}

connection InputFieldItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* InputFieldItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new InputFieldEditor(m_items, option.rect.width(), parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &InputFieldItemDelegate::on_editing_finished);
  return editor;
}

void InputFieldItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  m_item_modified_signal(index);
}

void InputFieldItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
