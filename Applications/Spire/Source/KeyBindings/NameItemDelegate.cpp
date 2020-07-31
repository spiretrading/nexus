#include "Spire/KeyBindings/NameItemDelegate.hpp"
#include <QFontMetrics>
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"
#include "Spire/Ui/TextInputWidget.hpp"

using namespace boost::signals2;
using namespace Spire;

NameItemDelegate::NameItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* NameItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new TextInputWidget(index.data().toString(),
    static_cast<QWidget*>(this->parent()));
  editor->set_cell_style();
  connect(editor, &TextInputWidget::editingFinished,
    this, &NameItemDelegate::on_editing_finished);
  return editor;
}

void NameItemDelegate::setEditorData(QWidget *editor,
    const QModelIndex &index) const {
  auto text_input = static_cast<TextInputWidget*>(editor);
  text_input->setText(index.data().value<QString>());
  text_input->setCursorPosition(text_input->text().length());
}

void NameItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto text = static_cast<QLineEdit*>(editor)->text().trimmed();
  model->setData(index, text, Qt::DisplayRole);
}
