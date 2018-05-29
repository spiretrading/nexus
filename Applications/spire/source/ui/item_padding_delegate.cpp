#include "spire/ui/item_padding_delegate.hpp"
#include <QPainter>

using namespace spire;

item_padding_delegate::item_padding_delegate(
  custom_variant_item_delegate* delegate, QObject* parent)
    : QStyledItemDelegate(parent),
      m_custom_variant_item_delegate(delegate) {}

QString item_padding_delegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_custom_variant_item_delegate->displayText(value, locale);
}

void item_padding_delegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->fillRect(option.rect,
    index.data(Qt::BackgroundRole).value<QColor>());
}

QSize item_padding_delegate::sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  return QSize();
}
