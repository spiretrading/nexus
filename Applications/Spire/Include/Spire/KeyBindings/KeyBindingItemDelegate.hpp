#ifndef SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#define SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Represents an item delegate for the KeyBindingsTableView.
  class KeyBindingItemDelegate : public QStyledItemDelegate {
    public:

      //! Signals that the item's data was modified.
      /*
        \param index The index that was modified.
      */
      using ItemModifiedSignal = Signal<void (const QModelIndex& index)>;

      //! Constructs a KeyBindingsItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit KeyBindingItemDelegate(QWidget* parent = nullptr);

      //! Connects a slot to the item modified signal.
      boost::signals2::connection connect_item_modified_signal(
        const ItemModifiedSignal::slot_type& slot) const;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void updateEditorGeometry(QWidget* editor,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    protected:
      mutable ItemModifiedSignal m_item_modified_signal;

      void on_editing_finished();
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      CustomVariantItemDelegate* m_item_delegate;
  };
}

#endif
