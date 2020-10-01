#include "Spire/KeyBindings/SideItemDelegate.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"
#include "Spire/Ui/SideComboBox.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideItemDelegate::SideItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* SideItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new SideComboBox(static_cast<QWidget*>(this->parent()));
  connect(editor, &SideComboBox::editingFinished,
    this, &SideItemDelegate::on_editing_finished);
  return editor;
}

void SideItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto side = static_cast<SideComboBox*>(editor);
  model->setData(index, QVariant::fromValue(side), Qt::DisplayRole);
}
