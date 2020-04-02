#ifndef SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#define SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#include <QStyledItemDelegate>
#include "Spire/KeyBindings/KeySequenceEditor.hpp"

namespace Spire {

  //! An item delegate for drawing and editing key sequences.
  class KeySequenceItemDelegate : public QStyledItemDelegate {
    public:

      //! Constructs a KeySequenceItemDelegate.
      /*
        \param The parent widget.
      */
	    explicit KeySequenceItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
    
      QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

    private:
      void draw_key_sequence(const QKeySequence& sequence, const QRect& rect,
        QPainter* painter) const;
      void draw_key(const QString& text, const QSize& text_size,
        const QPoint& pos, QPainter* painter) const;
      void on_editing_finished();
  };
}

#endif
