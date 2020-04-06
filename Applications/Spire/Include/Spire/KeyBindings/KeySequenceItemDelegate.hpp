#ifndef SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#define SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#include <QStyledItemDelegate>
#include "Spire/KeyBindings/KeySequenceEditor.hpp"

namespace Spire {

  //! An item delegate for drawing and editing key sequences.
  class KeySequenceItemDelegate : public QStyledItemDelegate {
    public:

      //! Signals that a table item's data was modified.
      /*
        \param row The row that was modified.
      */
      using ItemModifiedSignal = Signal<void (const QModelIndex& index)>;

      //! Constructs a KeySequenceItemDelegate.
      /*
        \param The parent widget.
      */
	    explicit KeySequenceItemDelegate(
        const std::vector<KeySequenceEditor::ValidKeySequence>&
        valid_key_sequences, QWidget* parent = nullptr);

      //! Connects a slot to the item modified signal.
      boost::signals2::connection connect_item_modified_signal(
        const ItemModifiedSignal::slot_type& slot) const;

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
    
      QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      mutable ItemModifiedSignal m_item_modified_signal;
       std::vector<KeySequenceEditor::ValidKeySequence> m_valid_key_sequences;
       mutable KeySequenceEditor* m_editor;

      void draw_key_sequence(const QKeySequence& sequence, const QRect& rect,
        QPainter* painter) const;
      void draw_key(const QString& text, const QSize& text_size,
        const QPoint& pos, QPainter* painter) const;
      void on_editing_finished();
  };
}

#endif
