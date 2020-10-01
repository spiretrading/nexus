#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Nexus/Definitions/OrderType.hpp"
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
  connect(editor, &OrderTypeComboBox::editingFinished,
    this, &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto type = static_cast<OrderTypeComboBox*>(editor)->get_order_type();
  model->setData(index, QVariant::fromValue(type), Qt::DisplayRole);
}
