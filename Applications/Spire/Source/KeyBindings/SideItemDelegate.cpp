#include "Spire/KeyBindings/SideItemDelegate.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "Spire/KeyBindings/InputFieldEditor.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

SideItemDelegate::SideItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent),
    m_item_delegate(new CustomVariantItemDelegate(this)) {}

connection SideItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

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

QString SideItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_item_delegate->displayText(value, locale);
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

void SideItemDelegate::updateEditorGeometry(QWidget* editor,
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

void SideItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
