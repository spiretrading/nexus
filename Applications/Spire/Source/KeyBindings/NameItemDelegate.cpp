#include "Spire/KeyBindings/NameItemDelegate.hpp"
#include <QLineEdit>
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Spire;

NameItemDelegate::NameItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* NameItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto str = index.data().value<QString>();
  auto editor = new QLineEdit("test", parent);
  editor->setStyleSheet(QString(R"(
    font-family: Roboto;
    font-size: %1px;
    padding-left: %2px;)").arg(scale_height(12)).arg(scale_width(5)));
  connect(editor, &QLineEdit::editingFinished,
    this, &NameItemDelegate::on_editing_finished);
  return editor;
}

void NameItemDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const {
  auto line_edit = static_cast<QLineEdit*>(editor);
  line_edit->setText(index.data().value<QString>());
  line_edit->setCursorPosition(line_edit->text().length());
}

void NameItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto text = static_cast<QLineEdit*>(editor)->text();
  model->setData(index, text, Qt::DisplayRole);
  m_item_modified_signal(index);
}
