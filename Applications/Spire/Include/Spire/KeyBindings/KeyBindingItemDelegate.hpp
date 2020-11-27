#ifndef SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#define SPIRE_KEY_BINDING_ITEM_DELEGATE_HPP
#include <QStyledItemDelegate>
#include "Spire/Ui/CustomQtVariants.hpp"

namespace Spire {

  //! Represents an item delegate for the KeyBindingsTableView.
  class KeyBindingItemDelegate : public QStyledItemDelegate {
    public:

      //! Represents the state of the delegate's editor when the editor was
      //! closed.
      enum class EditorState {

        //! The editor's input value has been accepted.
        ACCEPTED,

        //! The editor has been closed without submitting a value.
        CANCELLED,

        //! The editor has been closed and the cell's value should be deleted.
        DELETED
      };

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

      //! Returns the state of the delegate's editor.
      EditorState get_editor_state() const;

    protected:
      void on_editing_finished();
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      EditorState m_editor_state;
      CustomVariantItemDelegate* m_item_delegate;
  };
}

#endif
