#ifndef SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#define SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/Spire/Spire.hpp"
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Represents an item delegate for the KeyBindingsTableView.
  class KeyBindingItemDelegate : public QStyledItemDelegate {
    public:

      //! Signals that a key was filtered from the current editor.
      /*
        \param key The key that was pressed.
      */
      using KeySignal = Signal<void (Qt::Key key)>;

      //! Constructs a KeyBindingsItemDelegate.
      /*
        \param parent The parent widget.
      */
      explicit KeyBindingItemDelegate(QWidget* parent = nullptr);

      //! Connects a slot to the key signal.
      boost::signals2::connection connect_key_signal(
        const KeySignal::slot_type& slot) const;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void updateEditorGeometry(QWidget* editor,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    protected:
      void on_editing_finished();
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable KeySignal m_key_signal;
      CustomVariantItemDelegate* m_item_delegate;
  };
}

#endif
