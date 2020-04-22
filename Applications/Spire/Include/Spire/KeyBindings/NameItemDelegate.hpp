#ifndef SPIRE_NAME_ITEM_DELEGATE_HPP
#define SPIRE_NAME_ITEM_DELEGATE_HPP
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/Spire/Spire.hpp"

namespace Spire {

  class NameItemDelegate : public KeyBindingItemDelegate {
    public:

      explicit NameItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setEditorData(QWidget *editor,
        const QModelIndex &index) const override;

      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
  };
}

#endif
