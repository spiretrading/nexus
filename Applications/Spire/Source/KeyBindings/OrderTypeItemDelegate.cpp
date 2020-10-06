#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Spire/Ui/ComboBoxEditor.hpp"
#include "Spire/Ui/StaticDropDownMenu.hpp"

using namespace Spire;

OrderTypeItemDelegate::OrderTypeItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* OrderTypeItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto menu = new StaticDropDownMenu(make_order_type_list(),
    static_cast<QWidget*>(this->parent()));
  auto editor = new ComboBoxEditor(menu,
    static_cast<QWidget*>(this->parent()));
  editor->set_value(index.data());
  connect(editor, &ComboBoxEditor::editingFinished,
    this, &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto variant = [&] {
    auto combo_box = static_cast<ComboBoxEditor*>(editor);
    if(combo_box->get_last_key() == Qt::Key_Escape) {
      return index.data();
    } else if(combo_box->get_last_key() == Qt::Key_Delete) {
      return QVariant();
    }
    return combo_box->get_value();
  }();
  model->setData(index, variant, Qt::DisplayRole);
}
