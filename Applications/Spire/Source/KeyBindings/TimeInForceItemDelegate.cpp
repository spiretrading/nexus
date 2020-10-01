#include "Spire/KeyBindings/TimeInForceItemDelegate.hpp"
#include "Nexus/Definitions/TimeInForce.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/TimeInForceComboBox.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

TimeInForceItemDelegate::TimeInForceItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* TimeInForceItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return Spire::displayText(data.value<TimeInForce>());
    }
    return QString();
  }();
  auto editor = new TimeInForceComboBox(static_cast<QWidget*>(this->parent()));
  connect(editor, &TimeInForceComboBox::editingFinished,
    this, &TimeInForceItemDelegate::on_editing_finished);
  return editor;
}

void TimeInForceItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto item = static_cast<TimeInForceComboBox*>(
    editor)->get_current_time_in_force();
  model->setData(index, QVariant::fromValue(item), Qt::DisplayRole);
}
