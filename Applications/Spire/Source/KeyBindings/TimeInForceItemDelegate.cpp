#include "Spire/KeyBindings/TimeInForceItemDelegate.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

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

  auto get_time_in_force(const QString& text) {
    if(text == to_qstring(TimeInForce::Type::DAY)) {
      return TimeInForce::Type::DAY;
    } else if(text == to_qstring(TimeInForce::Type::FOK)) {
      return TimeInForce::Type::FOK;
    } else if(text == to_qstring(TimeInForce::Type::GTC)) {
      return TimeInForce::Type::GTC;
    } else if(text == to_qstring(TimeInForce::Type::GTD)) {
      return TimeInForce::Type::GTD;
    } else if(text == to_qstring(TimeInForce::Type::GTX)) {
      return TimeInForce::Type::GTX;
    } else if(text == to_qstring(TimeInForce::Type::IOC)) {
      return TimeInForce::Type::IOC;
    } else if(text == to_qstring(TimeInForce::Type::MOC)) {
      return TimeInForce::Type::MOC;
    } else if(text == to_qstring(TimeInForce::Type::OPG)) {
      return TimeInForce::Type::OPG;
    }
    return TimeInForce::Type::NONE;
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
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole).value<TimeInForce>().GetType();
    if(data != TimeInForce::Type::NONE) {
      return to_qstring(data);
    }
    return QString();
  }();
  qDebug() << current_data;
  auto editor = new InputFieldEditor(current_data,
    create_time_in_force_item_list(), parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &TimeInForceItemDelegate::on_editing_finished);
  return editor;
}

void TimeInForceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<InputFieldEditor*>(editor)->get_item();
  model->setData(index,
    QVariant::fromValue<TimeInForce>({get_time_in_force(item)}),
    Qt::DisplayRole);
  m_item_modified_signal(index);
}

void TimeInForceItemDelegate::updateEditorGeometry(QWidget* editor,
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

void TimeInForceItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
