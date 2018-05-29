#include "spire/ui/item_padding_delegate.hpp"

using namespace spire;

item_padding_delegate::item_padding_delegate(
  custom_variant_item_delegate* delegate, QObject* parent)
    : QStyledItemDelegate(parent),
      m_custom_variant_item_delegate(delegate) {}

QString item_padding_delegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_custom_variant_item_delegate->displayText(value, locale);
}
