#ifndef SPIRE_ORDER_TYPE_ITEM_DELEGATE_HPP
#define SPIRE_ORDER_TYPE_ITEM_DELEGATE_HPP
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QWidget>
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"

namespace Spire {

  //! Represents an item delegate for displaying and editing the order type of
  //! a key binding.
  class OrderTypeItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs an OrderTypeItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit OrderTypeItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
