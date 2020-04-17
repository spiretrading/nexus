#include "Spire/KeyBindings/QuantityItemDelegate.hpp"
#include <QIntValidator>
#include <QLineEdit>
#include "Nexus/Definitions/Quantity.hpp"
#include "Spire/Spire/Dimensions.hpp"

using namespace boost::signals2;
using namespace Nexus;
using namespace Spire;

QuantityItemDelegate::QuantityItemDelegate(QWidget* parent)
  : QStyledItemDelegate(parent),
    m_item_delegate(new CustomVariantItemDelegate(this)) {}

connection QuantityItemDelegate::connect_item_modified_signal(
    const ItemModifiedSignal::slot_type& slot) const {
  return m_item_modified_signal.connect(slot);
}

QWidget* QuantityItemDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  auto editor = new QLineEdit(parent);
  auto validator = new QIntValidator(editor);
  editor->setValidator(validator);
  editor->setStyleSheet(QString(R"(
    background-color: #FFFFFF;
    border: none;
    color: #000000;
    font-family: Roboto;
    font-size: %1px;
    padding-left: %2px;
  )").arg(scale_height(12)).arg(scale_width(8)));
  connect(editor, &QLineEdit::editingFinished,
    this, &QuantityItemDelegate::on_editing_finished);
  return editor;
}

QString QuantityItemDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_item_delegate->displayText(value, locale);
}

void QuantityItemDelegate::setModelData(QWidget* editor,
    QAbstractItemModel* model, const QModelIndex& index) const {
  auto ok = false;
  auto value = static_cast<QLineEdit*>(editor)->text().toInt(&ok);
  if(ok) {
    model->setData(index, QVariant::fromValue<Quantity>(value),
      Qt::DisplayRole);
    m_item_modified_signal(index);
  }
}

void QuantityItemDelegate::updateEditorGeometry(QWidget* editor,
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

void QuantityItemDelegate::on_editing_finished() {
  auto editor = static_cast<QWidget*>(sender());
  editor->close();
}
