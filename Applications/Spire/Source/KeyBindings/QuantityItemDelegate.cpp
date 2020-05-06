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
    static_cast<int>(index.data().value<Quantity>()), parent);
  connect(editor, &QLineEdit::editingFinished,
    this, &QuantityItemDelegate::on_editing_finished);
  return editor;
}

void QuantityItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto ok = false;
  auto value = static_cast<QLineEdit*>(editor)->text().toInt(&ok);
  if(ok) {
    model->setData(index, QVariant::fromValue<Quantity>(value),
      Qt::DisplayRole);
  }
}

bool QuantityItemDelegate::eventFilter(QObject* watched, QEvent* event) {
  if(event->type() == QEvent::KeyPress) {
    auto e = static_cast<QKeyEvent*>(event);
    if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
      return false;
    }
  }
  return KeyBindingItemDelegate::eventFilter(watched, event);
}
