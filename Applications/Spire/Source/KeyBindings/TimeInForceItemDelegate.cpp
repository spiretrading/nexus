#include "Spire/KeyBindings/TimeInForceItemDelegate.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

namespace {
  auto to_qstring(TimeInForce::Type type) {
    return QString::fromStdString(ToString(type));
  }

  auto create_time_in_force_item_list() {
    return std::vector<QString>({
        to_qstring(TimeInForce::Type::DAY),
        to_qstring(TimeInForce::Type::GTC),
        to_qstring(TimeInForce::Type::OPG),
        to_qstring(TimeInForce::Type::MOC),
        to_qstring(TimeInForce::Type::IOC),
        to_qstring(TimeInForce::Type::FOK),
        to_qstring(TimeInForce::Type::GTX),
        to_qstring(TimeInForce::Type::GTD)
      });
  }
}

TimeInForceItemDelegate::TimeInForceItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent) {}

connection TimeInForceItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* TimeInForceItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new InputFieldEditor(create_time_in_force_item_list(),
    option.rect.width(), parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &TimeInForceItemDelegate::on_editing_finished);
  return editor;
}

void TimeInForceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  m_item_modified_signal(index);
}

void TimeInForceItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
