#include "Spire/KeyBindings/NameItemDelegate.hpp"
#include "Spire/KeyBindings/NameInputEditor.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

NameItemDelegate::NameItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* NameItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto str = index.data().value<QString>();
  auto editor = new NameInputEditor(parent);
  connect(editor, &NameInputEditor::editingFinished,
    this, &NameItemDelegate::on_editing_finished);
  return editor;
}

void NameItemDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const {
  auto line_edit = static_cast<NameInputEditor*>(editor);
  line_edit->setText(index.data().value<QString>());
  line_edit->setCursorPosition(line_edit->text().length());
}

void NameItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto text = static_cast<QLineEdit*>(editor)->text().trimmed();
  model->setData(index, text, Qt::DisplayRole);
  m_item_modified_signal(index);
}
