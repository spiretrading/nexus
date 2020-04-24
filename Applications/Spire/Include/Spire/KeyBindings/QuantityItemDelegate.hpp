#ifndef SPIRE_QUANTITY_ITEM_DELEGATE_HPP
#define SPIRE_QUANTITY_ITEM_DELEGATE_HPP
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class QuantityItemDelegate : public KeyBindingItemDelegate {
    public:

      explicit QuantityItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
