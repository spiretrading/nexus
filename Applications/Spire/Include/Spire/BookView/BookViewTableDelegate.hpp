#ifndef SPIRE_BOOK_VIEW_TABLE_DELEGATE_HPP
#define SPIRE_BOOK_VIEW_TABLE_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/BookView/BookView.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Implements Qt's item delegate to support custom column padding.
  class BookViewTableDelegate : public QStyledItemDelegate {
    public:

      //! Constructs an item delegate for custom column padding.
      /*!
        \param delegate The delegate to add padding to.
        \param parent The parent to this delegate instance.
      */
      explicit BookViewTableDelegate(QStyledItemDelegate* delegate,
        QObject* parent = nullptr);

      QString displayText(const QVariant& value,
        const QLocale& locale) const override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    private:
      QStyledItemDelegate* m_item_delegate;
      CustomVariantItemDelegate* m_variant_delegate;
  };
}

#endif
