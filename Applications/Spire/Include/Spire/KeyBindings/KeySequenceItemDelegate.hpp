#ifndef SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#define SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#include "Spire/KeyBindings/KeyBindingItemDelegate.hpp"
#include "Spire/KeyBindings/KeySequenceEditor.hpp"

namespace Spire {

  //! An item delegate for drawing and editing key sequences.
  class KeySequenceItemDelegate : public KeyBindingItemDelegate {
    public:

      //! Constructs a KeySequenceItemDelegate.
      /*
        \param valid_key_sequences The list of accepted key sequences.
        \param parent The parent widget.
      */
      explicit KeySequenceItemDelegate(
        std::vector<KeySequenceEditor::ValidKeySequence> valid_key_sequences,
        QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;

    protected:
      bool eventFilter(QObject* watched, QEvent* event) override;

    private:
      std::vector<KeySequenceEditor::ValidKeySequence> m_valid_key_sequences;

      void draw_key_sequence(const QKeySequence& sequence, const QRect& rect,
        QPainter* painter) const;
      void draw_key(const QString& text, const QSize& text_size,
        const QPoint& pos, QPainter* painter) const;
  };
}

#endif
