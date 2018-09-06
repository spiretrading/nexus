#include "spire/book_view/book_view_table_delegate.hpp"
#include <QPainter>
#include "spire/spire/dimensions.hpp"

using namespace Spire;

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
  QStyleOptionViewItem opt(option);
  if(index.column() == 0) {
    painter->translate(QPoint(scale_width(5), 0));
  } else if(index.column() == 1 || index.column() == 2) {
    auto text_width = get_rendered_text_width(
      qvariant_cast<QFont>(index.data(Qt::FontRole)),
      m_variant_delegate->displayText(index.data(Qt::DisplayRole), QLocale()));
    auto text_pos = option.rect.width() - (text_width + scale_width(7));
    painter->translate(QPoint(text_pos, 0));
  }
  m_item_delegate->paint(painter, opt, index);
  painter->restore();
}

QSize BookViewTableDelegate::sizeHint(const QStyleOptionViewItem& option,
    const QModelIndex& index) const {
  return m_item_delegate->sizeHint(option, index);
}

int BookViewTableDelegate::get_rendered_text_width(const QFont& font,
    const QString& text) const {
  QFontMetrics metrics(font);
  return metrics.width(text);
}
