#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Spire/Ui/OrderTypeComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

OrderTypeItemDelegate::OrderTypeItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* OrderTypeItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new OrderTypeComboBox(true,
    static_cast<QWidget*>(this->parent()));
  editor->set_order_type(index.data().value<OrderType>());
  connect(editor, &OrderTypeComboBox::editingFinished, this,
    &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = [&] {
    if(get_editor_state() == EditorState::ACCEPTED) {
      return QVariant::fromValue(
        static_cast<OrderTypeComboBox*>(editor)->get_last_order_type());
    }
    return QVariant();
  }();
  model->setData(index, item, Qt::DisplayRole);
}
