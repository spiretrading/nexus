#ifndef SPIRE_DESTINATION_ITEM_DELEGATE_HPP
#define SPIRE_DESTINATION_ITEM_DELEGATE_HPP
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"

namespace Spire {

  //! Represents an item delegate for editing key binding destinations.
  class DestinationItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs a DestinationItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit DestinationItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
