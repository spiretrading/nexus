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
  m_initial_value = index.data();
  connect(editor, &OrderTypeComboBox::editingFinished,
    this, &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto variant = [&] {
    auto combo_box = static_cast<OrderTypeComboBox*>(editor);
    if(combo_box->get_last_key() == Qt::Key_Escape) {
      return m_initial_value;
    } else if(combo_box->get_last_key() == Qt::Key_Delete) {
      return QVariant();
    }
    return QVariant::fromValue(combo_box->get_order_type());
  }();
  model->setData(index, variant, Qt::DisplayRole);
}
