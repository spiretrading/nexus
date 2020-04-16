#include "Spire/KeyBindings/OrderTypeItemDelegate.hpp"
#include "Nexus/Definitions/OrderType.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_qstring(OrderType type) {
    return QString::fromStdString(ToString(type));
  }

  auto create_order_type_item_list() {
    static auto list = std::vector<QString>({
        to_qstring(OrderType::LIMIT),
        to_qstring(OrderType::MARKET),
        to_qstring(OrderType::PEGGED),
        to_qstring(OrderType::STOP)
      });
    return list;
  }

  auto get_order_type_variant(const QString& text) {
    if(text == to_qstring(OrderType::LIMIT)) {
      return QVariant::fromValue<OrderType>(OrderType::LIMIT);
    } else if(text == to_qstring(OrderType::MARKET)) {
      return QVariant::fromValue<OrderType>(OrderType::MARKET);
    } else if(text == to_qstring(OrderType::PEGGED)) {
      return QVariant::fromValue<OrderType>(OrderType::PEGGED);
    } else if(text == to_qstring(OrderType::STOP)) {
      return QVariant::fromValue<OrderType>(OrderType::STOP);
    }
    return QVariant();
  }
}

OrderTypeItemDelegate::OrderTypeItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent),
    m_item_delegate(new CustomVariantItemDelegate(this)) {}

connection OrderTypeItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* OrderTypeItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return to_qstring(data.value<OrderType>());
    }
    return QString();
  }();
  auto editor = new InputFieldEditor(current_data,
    create_order_type_item_list(), parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &OrderTypeItemDelegate::on_editing_finished);
  return editor;
}

QString OrderTypeItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_item_delegate->displayText(value, locale);
}

void OrderTypeItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item();
  model->setData(index, get_order_type_variant(item), Qt::DisplayRole);
  m_item_modified_signal(index);
}

void OrderTypeItemDelegate::updateEditorGeometry(QWidget* editor,
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

void OrderTypeItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
