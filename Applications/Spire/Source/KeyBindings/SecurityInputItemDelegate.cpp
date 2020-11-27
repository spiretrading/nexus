#include "Spire/KeyBindings/SecurityInputItemDelegate.hpp"
#include <QKeyEvent>
#include "Spire/SecurityInput/SecurityInputLineEdit.hpp"

using namespace Beam;
using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SecurityInputItemDelegate::SecurityInputItemDelegate(
  Ref<SecurityInputModel> model, QWidget* parent)
  : KeyBindingItemDelegate(parent),
    m_model(model.Get()) {}

QWidget* SecurityInputItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new SecurityInputLineEdit(index.data().value<Security>(),
    Ref<SecurityInputModel>(*m_model), false,
    static_cast<QWidget*>(this->parent()));
  connect(editor, &SecurityInputLineEdit::editingFinished, this,
    &SecurityInputItemDelegate::on_editing_finished);
  return editor;
}

void SecurityInputItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto security = [&] {
    if(get_editor_state() == EditorState::ACCEPTED) {
      return QVariant::fromValue(
        static_cast<SecurityInputLineEdit*>(editor)->get_security());
    }
    return QVariant();
  }();
  model->setData(index, security, Qt::DisplayRole);
}
