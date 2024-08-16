#ifndef SPIRE_STANDARDCANVASTABLEITEMDELEGATE_HPP
#define SPIRE_STANDARDCANVASTABLEITEMDELEGATE_HPP
#include <Beam/Pointers/Ref.hpp>
#include <QStyledItemDelegate>
#include "Spire/CanvasView/CanvasView.hpp"

namespace Spire {

  /*! \class StandardCanvasTableItemDelegate
      \brief Controls the drawing of a CanvasCell.
   */
  class StandardCanvasTableItemDelegate : public QStyledItemDelegate {
    public:

      //! Constructs a StandardCanvasTableItemDelegate.
      /*!
        \param parent The table displaying the item.
      */
      StandardCanvasTableItemDelegate(Beam::Ref<CanvasTableWidget> parent);

      void paint(QPainter* painter, const QStyleOptionViewItem& option,
        const QModelIndex& index) const;

    private:
      CanvasTableWidget* m_parent;
  };
}

#endif
