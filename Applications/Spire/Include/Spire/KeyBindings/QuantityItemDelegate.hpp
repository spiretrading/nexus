#ifndef SPIRE_QUANTITY_ITEM_DELEGATE_HPP
#define SPIRE_QUANTITY_ITEM_DELEGATE_HPP
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QWidget>
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Represents an item delegate for displaying and editing the quantity of a
  //! key binding.
  class QuantityItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs a QuantityItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit QuantityItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

    private:
      CustomVariantItemDelegate m_item_delegate;
  };
}

#endif
