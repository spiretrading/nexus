#include "Spire/KeyBindings/QuantityItemDelegate.hpp"
#include <QKeyEvent>
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/KeyBindings/QuantityInputEditor.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

QuantityItemDelegate::QuantityItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* QuantityItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new QuantityInputEditor(
    static_cast<int>(index.data().value<Quantity>()),
    static_cast<QWidget*>(this->parent()));
  connect(editor, &TextInputWidget::editingFinished,
    this, &QuantityItemDelegate::on_editing_finished);
  return editor;
}

void QuantityItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto quantity = [&] {
    if(get_editor_state() == EditorState::ACCEPTED) {
      auto ok = false;
      auto value = static_cast<TextInputWidget*>(editor)->text().toInt(&ok);
      if(ok) {
        return QVariant::fromValue<Quantity>(value);
      }
    }
    return QVariant();
  }();
  model->setData(index, quantity, Qt::DisplayRole);
}
