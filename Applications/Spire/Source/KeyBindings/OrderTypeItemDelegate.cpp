#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto& ORDER_TYPE_NAMES() {
    static auto list = std::vector<QVariant>({
      displayText(OrderType::LIMIT),
      displayText(OrderType::MARKET),
      displayText(OrderType::PEGGED),
      displayText(OrderType::STOP)
    });
    return list;
  }

  auto get_order_type_variant(const QString& text) {
    for(auto type : { OrderType::LIMIT, OrderType::MARKET, OrderType::PEGGED,
        OrderType::STOP }) {
      if(text == displayText(type)) {
        return QVariant::fromValue<OrderType>(type);
      }
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
  auto editor = new InputFieldEditor(QVariant::fromValue(current_data),
    ORDER_TYPE_NAMES(), static_cast<QWidget*>(this->parent()));
  connect(editor, &InputFieldEditor::editingFinished,
    this, &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(
    editor)->get_item().value<QString>().toUpper();
  model->setData(index, get_order_type_variant(item), Qt::DisplayRole);
}
