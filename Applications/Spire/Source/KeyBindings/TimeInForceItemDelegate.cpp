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
    static auto list = std::vector<QString>({
      to_qstring(TimeInForce::Type::DAY),
      to_qstring(TimeInForce::Type::FOK),
      to_qstring(TimeInForce::Type::GTC),
      to_qstring(TimeInForce::Type::GTD),
      to_qstring(TimeInForce::Type::GTX),
      to_qstring(TimeInForce::Type::IOC),
      to_qstring(TimeInForce::Type::MOC),
      to_qstring(TimeInForce::Type::OPG)
    });
    return list;
  }

  auto get_time_in_force_variant(const QString& text) {
    for(auto type : { TimeInForce::Type::DAY, TimeInForce::Type::FOK,
        TimeInForce::Type::GTC, TimeInForce::Type::GTD, TimeInForce::Type::GTX,
        TimeInForce::Type::IOC, TimeInForce::Type::MOC,
        TimeInForce::Type::OPG }) {
      if(text == to_qstring(type)) {
        return QVariant::fromValue<TimeInForce>({type});
      }
    }
    return QVariant();
  }
}

TimeInForceItemDelegate::TimeInForceItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* TimeInForceItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return to_qstring(data.value<TimeInForce>().GetType());
    }
    return QString();
  }();
  auto editor = new InputFieldEditor(current_data,
    create_time_in_force_item_list(), static_cast<QWidget*>(this->parent()));
  connect(editor, &InputFieldEditor::editingFinished,
    this, &TimeInForceItemDelegate::on_editing_finished);
  return editor;
}

void TimeInForceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item().toUpper();
  model->setData(index, get_time_in_force_variant(item), Qt::DisplayRole);
}
