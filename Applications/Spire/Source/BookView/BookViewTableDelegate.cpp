#include "Spire/BookView/BookViewTableDelegate.hpp"
#include <QPainter>
#include "Spire/Spire/Dimensions.hpp"

using namespace Spire;

namespace {
  auto calculate_rendered_text_width(const QFont& font, const QString& text) {
    auto metrics = QFontMetrics(font);
    return metrics.width(text);
  }
}

BookViewTableDelegate::BookViewTableDelegate(QStyledItemDelegate* delegate,
    QObject* parent)
    : QStyledItemDelegate(parent),
      m_item_delegate(delegate) {
  m_item_delegate->setParent(this);
  m_variant_delegate = new CustomVariantItemDelegate(this);
}

QString BookViewTableDelegate::displayText(const QVariant& value,
    const QLocale& locale) const {
  return m_item_delegate->displayText(value, locale);
}

void BookViewTableDelegate::paint(QPainter* painter,
    const QStyleOptionViewItem& option, const QModelIndex& index) const {
  painter->save();
  painter->fillRect(option.rect,
    index.data(Qt::BackgroundRole).value<QColor>());
  if(index.column() == 0) {
    painter->translate(QPoint(scale_width(5), 0));
  } else if(index.column() == 1 || index.column() == 2) {
    auto text_width = calculate_rendered_text_width(
      qvariant_cast<QFont>(index.data(Qt::FontRole)),
      m_variant_delegate->displayText(index.data(Qt::DisplayRole),
      option.locale));
    auto text_pos = option.rect.width() - (text_width + scale_width(7));
    painter->translate(QPoint(text_pos, 0));
  }
  m_item_delegate->paint(painter, option, index);
  painter->restore();
}

QSize BookViewTableDelegate::sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  return m_item_delegate->sizeHint(option, index);
}
