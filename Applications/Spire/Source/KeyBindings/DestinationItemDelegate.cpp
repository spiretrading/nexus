#include "Spire/KeyBindings/DestinationItemDelegate.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Spire;


DestinationItemDelegate::DestinationItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent),
    m_item_delegate(new CustomVariantItemDelegate(this)) {}

connection DestinationItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* DestinationItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new InputFieldEditor("", {"One", "Two", "Three"}, parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &DestinationItemDelegate::on_editing_finished);
  return editor;
}

QString DestinationItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_item_delegate->displayText(value, locale);
}

void DestinationItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item();
  model->setData(index, QVariant(item), Qt::DisplayRole);
  m_item_modified_signal(index);
}

void DestinationItemDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  if(index.row() == 0) {
    auto rect = option.rect.translated(0, 1);
    editor->move(rect.topLeft());
    rect.setHeight(rect.height() - 1);
    editor->resize(rect.size());
  } else {
    editor->move(option.rect.topLeft());
    editor->resize(option.rect.size());
  }
}

void DestinationItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
