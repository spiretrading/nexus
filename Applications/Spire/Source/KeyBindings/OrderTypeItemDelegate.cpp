#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto create_order_type_item_list() {
    static auto list = std::vector<QString>({
        displayText(OrderType::LIMIT),
        displayText(OrderType::MARKET),
        displayText(OrderType::PEGGED),
        displayText(OrderType::STOP)
      });
    return list;
  }

  auto get_order_type_variant(const QString& text) {
    if(text == displayText(OrderType::LIMIT)) {
      return QVariant::fromValue<OrderType>(OrderType::LIMIT);
    } else if(text == displayText(OrderType::MARKET)) {
      return QVariant::fromValue<OrderType>(OrderType::MARKET);
    } else if(text == displayText(OrderType::PEGGED)) {
      return QVariant::fromValue<OrderType>(OrderType::PEGGED);
    } else if(text == displayText(OrderType::STOP)) {
      return QVariant::fromValue<OrderType>(OrderType::STOP);
    }
    return QVariant();
  }
}

OrderTypeItemDelegate::OrderTypeItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* OrderTypeItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return Spire::displayText(data.value<OrderType>());
    }
    return QString();
  }();
  auto editor = new InputFieldEditor(current_data,
    create_order_type_item_list(), parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item().toUpper();
  model->setData(index, get_order_type_variant(item), Qt::DisplayRole);
}
