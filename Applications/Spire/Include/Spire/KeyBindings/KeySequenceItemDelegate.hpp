#ifndef SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#define SPIRE_KEY_SEQUENCE_ITEM_DELEGATE
#include <QAbstractItemDelegate>

namespace Spire {

  //! An item delegate for drawing and editing key sequences.
  class KeySequenceItemDelegate : public QAbstractItemDelegate {
    public:

      //! Constructs a KeySequenceItemDelegate.
      /*
        \param The parent widget.
      */
	    explicit KeySequenceItemDelegate(QWidget* parent = nullptr);

      QWidget* createEditor(QWidget* parent,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      bool editorEvent(QEvent* event, QAbstractItemModel* model,
        const QStyleOptionViewItem& option, const QModelIndex& index) override;

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void setEditorData(QWidget* editor,
        const QModelIndex& index) const override;
    
      void setModelData(QWidget* editor, QAbstractItemModel* model,
        const QModelIndex& index) const override;
    
      QSize sizeHint(const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;

      void updateEditorGeometry(QWidget* editor,
        const QStyleOptionViewItem& option,
        const QModelIndex& index) const override;
  };
}

#endif
