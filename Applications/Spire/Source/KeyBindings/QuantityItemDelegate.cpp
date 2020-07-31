#include "Spire/KeyBindings/QuantityItemDelegate.hpp"
#include <QKeyEvent>
#include <QPainter>
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
  connect(editor, &QLineEdit::editingFinished,
    this, &QuantityItemDelegate::on_editing_finished);
  return editor;
}

void QuantityItemDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect,
    index.data(Qt::BackgroundRole).value<QColor>());
  auto font = QFont("Roboto");
  font.setPixelSize(scale_height(12));
  painter->setFont(font);
  auto metrics = QFontMetrics(font);
  auto text = metrics.elidedText(
    m_item_delegate.displayText(index.data()), Qt::ElideRight,
    option.rect.width() - scale_width(8));
  auto pos = QPoint(option.rect.left() + scale_width(8),
    option.rect.bottom() - (option.rect.height() - metrics.ascent()) / 2);
  painter->drawText(pos, text);
  painter->restore();
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
