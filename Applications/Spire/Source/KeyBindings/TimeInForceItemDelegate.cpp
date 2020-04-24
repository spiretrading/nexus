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
    if(text == to_qstring(TimeInForce::Type::DAY)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::DAY});
    } else if(text == to_qstring(TimeInForce::Type::FOK)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::FOK});
    } else if(text == to_qstring(TimeInForce::Type::GTC)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::GTC});
    } else if(text == to_qstring(TimeInForce::Type::GTD)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::GTD});
    } else if(text == to_qstring(TimeInForce::Type::GTX)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::GTX});
    } else if(text == to_qstring(TimeInForce::Type::IOC)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::IOC});
    } else if(text == to_qstring(TimeInForce::Type::MOC)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::MOC});
    } else if(text == to_qstring(TimeInForce::Type::OPG)) {
      return QVariant::fromValue<TimeInForce>({TimeInForce::Type::OPG});
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
    create_time_in_force_item_list(), parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &TimeInForceItemDelegate::on_editing_finished);
  return editor;
}

void TimeInForceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item().toUpper();
  model->setData(index, get_time_in_force_variant(item), Qt::DisplayRole);
  m_item_modified_signal(index);
}
