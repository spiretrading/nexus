#include "Spire/KeyBindings/SecurityInputItemDelegate.hpp"
#include "Spire/SecurityInput/SecurityInputBox.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Spire;

SecurityInputItemDelegate::SecurityInputItemDelegate(
  Ref<SecurityInputModel> model, QWidget* parent)
  : QStyledItemDelegate(parent),
    m_model(model.Get()) {}

connection SecurityInputItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* SecurityInputItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new SecurityInputBox(Ref<SecurityInputModel>(*m_model),
    parent);
  //connect(editor, &SecurityInputBox::editingFinished,
  //  this, &InputFieldItemDelegate::on_editing_finished);
  return editor;
}

void SecurityInputItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  m_item_modified_signal(index);
}

void SecurityInputItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
