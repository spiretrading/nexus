#include "Spire/KeyBindings/SideItemDelegate.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideItemDelegate::SideItemDelegate(QWidget* parent)
  : KeyBindingItemDelegate(parent) {}

QWidget* SideItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto current_data = [&] {
    auto data = index.data(Qt::DisplayRole);
    if(data.isValid()) {
      return displayText(data, QLocale());
    }
    return QString();
  }();
  auto editor = new InputFieldEditor(current_data,
    {displayText(Side::ASK, QLocale()), displayText(Side::BID, QLocale())},
    parent);
  connect(editor, &InputFieldEditor::editingFinished,
    this, &SideItemDelegate::on_editing_finished);
  return editor;
}

void SideItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto variant = [&] {
    auto item = static_cast<InputFieldEditor*>(editor)->get_item();
    if(item == displayText(Side::BID, QLocale())) {
      return QVariant::fromValue<Side>(Side::BID);
    } else if(item == displayText(Side::ASK, QLocale())) {
      return QVariant::fromValue<Side>(Side::ASK);
    }
    return QVariant();
  }();
  model->setData(index, variant, Qt::DisplayRole);
  m_item_modified_signal(index);
}
