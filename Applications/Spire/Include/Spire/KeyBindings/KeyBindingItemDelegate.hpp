#ifndef SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#define SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Represents an item delegate for the KeyBindingsTableView.
  class KeyBindingItemDelegate : public QStyledItemDelegate {
    public:

      //! Constructs a KeyBindingsItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit KeyBindingItemDelegate(QWidget* parent = nullptr);

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void setEditorData(QWidget* editor,
        const QModelIndex& index) const override;

      void updateEditorGeometry(QWidget* editor,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    protected:
      void on_editing_finished();
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      CustomVariantItemDelegate* m_item_delegate;
  };
}

#endif
