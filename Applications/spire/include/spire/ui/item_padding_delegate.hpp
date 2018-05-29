#ifndef SPIRE_ITEM_PADDING_DELEGATE
#define SPIRE_ITEM_PADDING_DELEGATE
#include <QStyledItemDelegate>
#include "spire/ui/custom_qt_variants.hpp"

namespace spire {

  //! Implements Qt' item delegate to support custom item padding.
  class item_padding_delegate : public QStyledItemDelegate {
    public:

      //! Constructs an item delegate for custom padding.
      item_padding_delegate(custom_variant_item_delegate* delegate,
        QObject* parent = nullptr);

      //! Forwards the returned value from custom_variant_item_delegate
      QString displayText(const QVariant& value,
        const QLocale& locale) const override;

    private:
      custom_variant_item_delegate* m_custom_variant_item_delegate;
  };
}

#endif
