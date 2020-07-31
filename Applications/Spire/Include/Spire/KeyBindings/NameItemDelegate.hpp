#ifndef SPIRE_NAME_ITEM_DELEGATE_HPP
#define SPIRE_NAME_ITEM_DELEGATE_HPP
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"

namespace Spire {

  //! Represents an item delegate for displaying and editing the name of a key
  //! binding.
  class NameItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs a NameItemDelegate.
      /*
        \parent The parent widget.
      */
      explicit NameItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void paint(QPainter* painter,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void setEditorData(QWidget *editor,
        const QModelIndex &index) const override;

      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
